package com.tempolink.roomservice.signaling;

import org.springframework.stereotype.Component;
import org.springframework.web.socket.WebSocketSession;

import java.util.ArrayList;
import java.util.List;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;
import java.util.stream.Collectors;

@Component
public class SignalingSessionRegistry {
  public record SessionContext(String sessionId, String roomCode, String userId,
                               WebSocketSession session) {
  }

  private final ConcurrentMap<String, SessionContext> sessionsById = new ConcurrentHashMap<>();

  public void register(WebSocketSession session, String roomCode, String userId) {
    sessionsById.put(session.getId(), new SessionContext(session.getId(), roomCode, userId, session));
  }

  public void unregister(String sessionId) {
    sessionsById.remove(sessionId);
  }

  public SessionContext findBySessionId(String sessionId) {
    return sessionsById.get(sessionId);
  }

  public List<SessionContext> sessionsInRoom(String roomCode) {
    List<SessionContext> result = new ArrayList<>();
    for (SessionContext context : sessionsById.values()) {
      if (context.roomCode().equals(roomCode)) {
        result.add(context);
      }
    }
    return result;
  }

  public List<SessionContext> sessionsForUser(String roomCode, String userId) {
    List<SessionContext> result = new ArrayList<>();
    for (SessionContext context : sessionsById.values()) {
      if (context.roomCode().equals(roomCode) && context.userId().equals(userId)) {
        result.add(context);
      }
    }
    return result;
  }

  public Set<String> usersInRoom(String roomCode) {
    return sessionsInRoom(roomCode).stream()
        .map(SessionContext::userId)
        .collect(Collectors.toSet());
  }
}

