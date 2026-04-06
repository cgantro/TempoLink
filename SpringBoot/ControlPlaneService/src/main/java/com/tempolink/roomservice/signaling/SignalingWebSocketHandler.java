package com.tempolink.roomservice.signaling;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.tempolink.roomservice.dto.RoomResponse;
import com.tempolink.roomservice.service.RoomService;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.scheduling.annotation.Scheduled;
import org.springframework.stereotype.Component;
import org.springframework.web.socket.CloseStatus;
import org.springframework.web.socket.TextMessage;
import org.springframework.web.socket.WebSocketSession;
import org.springframework.web.socket.handler.TextWebSocketHandler;
import org.springframework.web.util.UriComponentsBuilder;

import java.io.IOException;
import java.net.URI;
import java.util.List;
import java.util.Map;
import java.util.Set;

@Component
public class SignalingWebSocketHandler extends TextWebSocketHandler {
  private final ObjectMapper objectMapper;
  private final RoomService roomService;
  private final SignalingSessionRegistry sessionRegistry;
  private final long sessionIdleTimeoutMillis;

  public SignalingWebSocketHandler(ObjectMapper objectMapper,
                                   RoomService roomService,
                                   SignalingSessionRegistry sessionRegistry,
                                   @Value("${tempolink.signaling.session-idle-timeout-ms:30000}")
                                   long sessionIdleTimeoutMillis) {
    this.objectMapper = objectMapper;
    this.roomService = roomService;
    this.sessionRegistry = sessionRegistry;
    this.sessionIdleTimeoutMillis = sessionIdleTimeoutMillis;
  }

  @Override
  public void afterConnectionEstablished(WebSocketSession session) throws IOException {
    final String roomCode = queryParam(session.getUri(), "roomCode");
    final String userId = queryParam(session.getUri(), "userId");

    if (roomCode == null || roomCode.isBlank() || userId == null || userId.isBlank()) {
      session.close(CloseStatus.BAD_DATA.withReason("roomCode and userId are required"));
      return;
    }

    final RoomResponse joinedRoom;
    try {
      joinedRoom = roomService.join(roomCode, userId);
    } catch (RuntimeException ex) {
      session.close(CloseStatus.POLICY_VIOLATION.withReason("unable to join room"));
      return;
    }

    sessionRegistry.register(session, roomCode, userId);
    Set<String> participants = Set.copyOf(joinedRoom.participants());

    sendJson(session, new SignalingEnvelope(
        "room.joined",
        roomCode,
        "server",
        userId,
        Map.of("participants", participants)
    ));

    broadcastToRoom(roomCode, session.getId(), new SignalingEnvelope(
        "peer.joined",
        roomCode,
        userId,
        null,
        Map.of("userId", userId)
    ));
  }

  @Override
  protected void handleTextMessage(WebSocketSession session, TextMessage message) throws IOException {
    SignalingSessionRegistry.SessionContext source = sessionRegistry.findBySessionId(session.getId());
    if (source == null) {
      session.close(CloseStatus.POLICY_VIOLATION.withReason("session not registered"));
      return;
    }
    sessionRegistry.touch(session.getId());

    final SignalingEnvelope incoming;
    try {
      incoming = objectMapper.readValue(message.getPayload(), SignalingEnvelope.class);
    } catch (JsonProcessingException ex) {
      sendJson(session, new SignalingEnvelope(
          "signal.error",
          source.roomCode(),
          "server",
          source.userId(),
          Map.of("message", "invalid signaling payload")
      ));
      return;
    }

    SignalingEnvelope forwarded = new SignalingEnvelope(
        incoming.type(),
        source.roomCode(),
        source.userId(),
        incoming.toUserId(),
        incoming.payload()
    );

    if (incoming.toUserId() == null || incoming.toUserId().isBlank()) {
      broadcastToRoom(source.roomCode(), session.getId(), forwarded);
      return;
    }

    List<SignalingSessionRegistry.SessionContext> targets =
        sessionRegistry.sessionsForUser(source.roomCode(), incoming.toUserId());
    for (SignalingSessionRegistry.SessionContext target : targets) {
      sendJson(target.session(), forwarded);
    }
  }

  @Override
  public void afterConnectionClosed(WebSocketSession session, CloseStatus status) throws IOException {
    SignalingSessionRegistry.SessionContext source = sessionRegistry.unregister(session.getId());
    if (source == null) {
      return;
    }
    leaveRoomAndBroadcastPeerLeft(source, session.getId());
  }

  @Scheduled(fixedDelayString = "${tempolink.signaling.cleanup-interval-ms:10000}")
  public void cleanupStaleSessions() {
    List<SignalingSessionRegistry.SessionContext> staleSessions =
        sessionRegistry.findStaleSessions(sessionIdleTimeoutMillis);
    for (SignalingSessionRegistry.SessionContext stale : staleSessions) {
      SignalingSessionRegistry.SessionContext removed =
          sessionRegistry.unregister(stale.sessionId());
      if (removed == null) {
        continue;
      }
      try {
        if (removed.session().isOpen()) {
          removed.session().close(CloseStatus.GOING_AWAY.withReason("session timeout"));
        }
      } catch (IOException ignored) {
        // Ignore socket close errors and continue with room cleanup.
      }

      try {
        leaveRoomAndBroadcastPeerLeft(removed, removed.sessionId());
      } catch (IOException ignored) {
        // Best-effort cleanup; next state refresh will reconcile participants.
      }
    }
  }

  private void leaveRoomAndBroadcastPeerLeft(SignalingSessionRegistry.SessionContext source,
                                             String excludeSessionId) throws IOException {
    try {
      roomService.leave(source.roomCode(), source.userId());
    } catch (RuntimeException ignored) {
      // Room may already be removed or not found; signaling close should still proceed.
    }
    broadcastToRoom(source.roomCode(), excludeSessionId, new SignalingEnvelope(
        "peer.left",
        source.roomCode(),
        source.userId(),
        null,
        Map.of("userId", source.userId())
    ));
  }

  private void broadcastToRoom(String roomCode, String excludeSessionId,
                               SignalingEnvelope envelope) throws IOException {
    for (SignalingSessionRegistry.SessionContext target : sessionRegistry.sessionsInRoom(roomCode)) {
      if (target.sessionId().equals(excludeSessionId)) {
        continue;
      }
      sendJson(target.session(), envelope);
    }
  }

  private void sendJson(WebSocketSession session, SignalingEnvelope envelope) throws IOException {
    if (!session.isOpen()) {
      return;
    }
    String payload = objectMapper.writeValueAsString(envelope);
    session.sendMessage(new TextMessage(payload));
  }

  private String queryParam(URI uri, String key) {
    if (uri == null) {
      return null;
    }
    return UriComponentsBuilder.fromUri(uri).build().getQueryParams().getFirst(key);
  }
}
