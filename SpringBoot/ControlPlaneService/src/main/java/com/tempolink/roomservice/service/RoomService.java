package com.tempolink.roomservice.service;

import com.tempolink.roomservice.domain.Room;
import com.tempolink.roomservice.dto.CreateRoomRequest;
import com.tempolink.roomservice.dto.RoomResponse;
import com.tempolink.roomservice.exception.RoomCapacityExceededException;
import com.tempolink.roomservice.exception.RoomNotFoundException;
import org.springframework.stereotype.Service;

import java.security.SecureRandom;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

@Service
public class RoomService {
  private static final String ROOM_CHARS = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";
  private final SecureRandom random = new SecureRandom();
  private final Map<String, Room> rooms = new ConcurrentHashMap<>();

  public RoomResponse create(CreateRoomRequest request) {
    final int maxParticipants = request.maxParticipants() == null ? 4 : request.maxParticipants();
    final String relayHost = (request.relayHost() == null || request.relayHost().isBlank())
        ? "127.0.0.1" : request.relayHost();
    final int relayPort = request.relayPort() == null ? 40000 : request.relayPort();

    String roomCode = generateUniqueRoomCode();
    Room room = new Room(roomCode, request.hostUserId(), maxParticipants, relayHost, relayPort);
    rooms.put(roomCode, room);
    return toResponse(room);
  }

  public List<RoomResponse> list() {
    return rooms.values().stream()
        .map(this::toResponse)
        .sorted(Comparator.comparing(RoomResponse::createdAt).reversed())
        .toList();
  }

  public RoomResponse get(String roomCode) {
    Room room = rooms.get(roomCode);
    if (room == null) {
      throw new RoomNotFoundException(roomCode);
    }
    return toResponse(room);
  }

  public RoomResponse join(String roomCode, String userId) {
    Room room = rooms.get(roomCode);
    if (room == null) {
      throw new RoomNotFoundException(roomCode);
    }

    synchronized (room) {
      if (!room.participants().contains(userId) &&
          room.participants().size() >= room.maxParticipants()) {
        throw new RoomCapacityExceededException(roomCode);
      }
      room.participants().add(userId);
      return toResponse(room);
    }
  }

  public RoomResponse leave(String roomCode, String userId) {
    Room room = rooms.get(roomCode);
    if (room == null) {
      throw new RoomNotFoundException(roomCode);
    }

    synchronized (room) {
      room.participants().remove(userId);
      if (room.participants().isEmpty()) {
        rooms.remove(roomCode);
        return new RoomResponse(
            room.roomCode(),
            room.hostUserId(),
            room.maxParticipants(),
            0,
            List.of(),
            room.relayHost(),
            room.relayPort(),
            room.createdAt()
        );
      }
      return toResponse(room);
    }
  }

  private RoomResponse toResponse(Room room) {
    return new RoomResponse(
        room.roomCode(),
        room.hostUserId(),
        room.maxParticipants(),
        room.participants().size(),
        new ArrayList<>(room.participants()),
        room.relayHost(),
        room.relayPort(),
        room.createdAt()
    );
  }

  private String generateUniqueRoomCode() {
    String roomCode;
    do {
      roomCode = randomRoomCode(6);
    } while (rooms.containsKey(roomCode));
    return roomCode;
  }

  private String randomRoomCode(int length) {
    StringBuilder sb = new StringBuilder(length);
    for (int i = 0; i < length; i++) {
      sb.append(ROOM_CHARS.charAt(random.nextInt(ROOM_CHARS.length())));
    }
    return sb.toString();
  }
}
