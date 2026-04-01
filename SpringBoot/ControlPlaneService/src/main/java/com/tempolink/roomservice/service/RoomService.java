package com.tempolink.roomservice.service;

import com.tempolink.roomservice.common.ServiceConstants;
import com.tempolink.roomservice.config.RelayProperties;
import com.tempolink.roomservice.domain.Room;
import com.tempolink.roomservice.domain.RoomRepository;
import com.tempolink.roomservice.dto.CreateRoomRequest;
import com.tempolink.roomservice.dto.DeleteRoomRequest;
import com.tempolink.roomservice.dto.RoomResponse;
import com.tempolink.roomservice.dto.UpdateRoomRequest;
import com.tempolink.roomservice.exception.RoomCapacityExceededException;
import com.tempolink.roomservice.exception.RoomNotFoundException;
import com.tempolink.roomservice.exception.RoomPermissionDeniedException;
import jakarta.annotation.PostConstruct;
import lombok.RequiredArgsConstructor;

import org.springframework.dao.DataIntegrityViolationException;
import org.springframework.data.domain.Sort;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.security.SecureRandom;
import java.util.ArrayList;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Set;
import java.util.stream.Collectors;

@Service
@RequiredArgsConstructor
public class RoomService {
  private static final String ROOM_CHARS = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";
  private static final int MAX_ROOM_CODE_GENERATION_ATTEMPTS = 32;

  private final SecureRandom random = new SecureRandom();
  private final RoomRepository roomRepository;
  private final RelayProperties relayProperties;

  @PostConstruct
  void init() {
    ensureTestRoom();
  }

  @Transactional
  public RoomResponse create(CreateRoomRequest request) {
    final int maxParticipants = normalizeMaxParticipants(request.maxParticipants());
    final String relayHost = (request.relayHost() == null || request.relayHost().isBlank())
        ? relayProperties.getHost()
        : request.relayHost();
    final int relayPort = request.relayPort() == null
        ? relayProperties.getPort()
        : request.relayPort();
    final boolean hasPassword = Boolean.TRUE.equals(request.hasPassword())
        && request.password() != null
        && !request.password().isBlank();

    String roomCode = generateUniqueRoomCode();
    Room room = new Room(
        roomCode,
        request.hostUserId(),
        normalizeName(request.name(), roomCode),
        normalizeDescription(request.description()),
        normalizeTags(request.tags()),
        request.isPublic() == null || request.isPublic(),
        hasPassword,
        hasPassword ? request.password() : null,
        maxParticipants,
        relayHost,
        relayPort);
    roomRepository.save(room);
    return toResponse(room);
  }

  @Transactional(readOnly = true)
  public List<RoomResponse> list(String query, String tag, Boolean isPublic,
      Boolean hasPassword, String mode) {
    final String normalizedQuery = query == null ? "" : query.trim().toLowerCase();
    final String normalizedTag = tag == null ? "" : tag.trim().toLowerCase();
    final String normalizedMode = mode == null ? "all" : mode.trim().toLowerCase();

    return roomRepository.findAll(Sort.by(Sort.Direction.DESC, "createdAt")).stream()
        .filter(room -> matchesQuery(room, normalizedQuery))
        .filter(room -> matchesTag(room, normalizedTag))
        .filter(room -> isPublic == null || room.isPublic() == isPublic)
        .filter(room -> hasPassword == null || room.hasPassword() == hasPassword)
        .filter(room -> matchesMode(room, normalizedMode))
        .map(this::toResponse)
        .toList();
  }

  @Transactional(readOnly = true)
  public RoomResponse get(String roomCode) {
    return toResponse(findRoom(roomCode));
  }

  @Transactional
  public RoomResponse join(String roomCode, String userId) {
    Room room = findRoom(roomCode);
    Set<String> participants = room.participants();

    if (!participants.contains(userId) && participants.size() >= room.maxParticipants()) {
      throw new RoomCapacityExceededException(roomCode);
    }
    participants.add(userId);
    return toResponse(room);
  }

  @Transactional
  public RoomResponse leave(String roomCode, String userId) {
    Room room = findRoom(roomCode);

    room.participants().remove(userId);
    if (room.participants().isEmpty()) {
      if (ServiceConstants.TEST_ROOM_CODE.equals(roomCode)) {
        room.participants().add(ServiceConstants.TEST_ROOM_HOST_USER_ID);
        return toResponse(room);
      }
      roomRepository.delete(room);
      return toResponse(room);
    }
    return toResponse(room);
  }

  @Transactional
  public RoomResponse update(String roomCode, UpdateRoomRequest request) {
    Room room = findRoom(roomCode);
    assertHostOwner(room, request.hostUserId());

    final boolean hasPassword = Boolean.TRUE.equals(request.hasPassword())
        && request.password() != null
        && !request.password().isBlank();
    room.updateMetadata(
        normalizeName(request.name(), room.roomCode()),
        normalizeDescription(request.description()),
        normalizeTags(request.tags()),
        request.isPublic() == null || request.isPublic(),
        hasPassword,
        hasPassword ? request.password() : null,
        normalizeMaxParticipants(request.maxParticipants()));
    return toResponse(room);
  }

  @Transactional
  public void delete(String roomCode, DeleteRoomRequest request) {
    Room room = findRoom(roomCode);
    assertHostOwner(room, request.hostUserId());
    if (ServiceConstants.TEST_ROOM_CODE.equals(roomCode)) {
      throw new IllegalArgumentException("TestRoom cannot be deleted");
    }
    roomRepository.delete(room);
  }

  private RoomResponse toResponse(Room room) {
    List<String> participants = new ArrayList<>(room.participants()).stream().sorted().toList();
    List<String> tags = new ArrayList<>(room.tags()).stream().sorted().toList();
    return new RoomResponse(
        room.roomCode(),
        room.hostUserId(),
        room.name(),
        room.description(),
        tags,
        room.isPublic(),
        room.hasPassword(),
        room.maxParticipants(),
        participants.size(),
        participants,
        room.relayHost(),
        room.relayPort(),
        room.createdAt());
  }

  private String generateUniqueRoomCode() {
    for (int i = 0; i < MAX_ROOM_CODE_GENERATION_ATTEMPTS; i++) {
      String roomCode = randomRoomCode(6);
      if (!roomRepository.existsByRoomCode(roomCode)) {
        return roomCode;
      }
    }
    throw new IllegalStateException("Unable to generate unique room code");
  }

  private String randomRoomCode(int length) {
    StringBuilder sb = new StringBuilder(length);
    for (int i = 0; i < length; i++) {
      sb.append(ROOM_CHARS.charAt(random.nextInt(ROOM_CHARS.length())));
    }
    return sb.toString();
  }

  private Room findRoom(String roomCode) {
    return roomRepository.findByRoomCode(roomCode)
        .orElseThrow(() -> new RoomNotFoundException(roomCode));
  }

  private void assertHostOwner(Room room, String hostUserId) {
    if (!room.hostUserId().equals(hostUserId)) {
      throw new RoomPermissionDeniedException(room.roomCode());
    }
  }

  private static int normalizeMaxParticipants(Integer maxParticipants) {
    if (maxParticipants == null) {
      return ServiceConstants.DEFAULT_MAX_PARTICIPANTS;
    }
    return Math.max(2, Math.min(ServiceConstants.MAX_ROOM_PARTICIPANTS, maxParticipants));
  }

  private static String normalizeName(String requestedName, String roomCode) {
    if (requestedName == null || requestedName.isBlank()) {
      return "Room " + roomCode;
    }
    return requestedName.trim();
  }

  private static String normalizeDescription(String requestedDescription) {
    return requestedDescription == null ? "" : requestedDescription.trim();
  }

  private static Set<String> normalizeTags(List<String> requestedTags) {
    if (requestedTags == null || requestedTags.isEmpty()) {
      return Set.of();
    }
    return requestedTags.stream()
        .filter(tag -> tag != null && !tag.isBlank())
        .map(String::trim)
        .limit(8)
        .collect(Collectors.toCollection(LinkedHashSet::new));
  }

  private static boolean matchesQuery(Room room, String query) {
    if (query.isEmpty()) {
      return true;
    }
    if (room.roomCode().toLowerCase().contains(query)) {
      return true;
    }
    if (room.name().toLowerCase().contains(query)) {
      return true;
    }
    if (room.description().toLowerCase().contains(query)) {
      return true;
    }
    return room.tags().stream().anyMatch(tag -> tag.toLowerCase().contains(query));
  }

  private static boolean matchesTag(Room room, String tag) {
    if (tag.isEmpty()) {
      return true;
    }
    return room.tags().stream().anyMatch(t -> t.equalsIgnoreCase(tag));
  }

  private static boolean matchesMode(Room room, String mode) {
    final boolean active = !room.participants().isEmpty();
    return switch (mode) {
      case "active" -> active;
      case "standby" -> !active;
      default -> true;
    };
  }

  private void ensureTestRoom() {
    if (roomRepository.existsByRoomCode(ServiceConstants.TEST_ROOM_CODE)) {
      return;
    }
    try {
      Room room = new Room(
          ServiceConstants.TEST_ROOM_CODE,
          ServiceConstants.TEST_ROOM_HOST_USER_ID,
          "TestRoom",
          "Always-on test room for audio/network checks",
          Set.of("test", "network", "audio"),
          true,
          false,
          null,
          ServiceConstants.TEST_ROOM_MAX_PARTICIPANTS,
          relayProperties.getHost(),
          relayProperties.getPort());
      room.participants().add(ServiceConstants.TEST_ROOM_HOST_USER_ID);
      roomRepository.save(room);
    } catch (DataIntegrityViolationException ignored) {
      // Another request may create the test room at the same time.
    }
  }
}
