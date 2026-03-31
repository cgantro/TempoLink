package com.tempolink.roomservice.dto;

import java.time.Instant;
import java.util.List;

public record RoomResponse(
    String roomCode,
    String hostUserId,
    int maxParticipants,
    int participantCount,
    List<String> participants,
    String relayHost,
    int relayPort,
    Instant createdAt
) {
}

