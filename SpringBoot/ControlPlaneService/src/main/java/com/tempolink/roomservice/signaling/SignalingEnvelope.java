package com.tempolink.roomservice.signaling;

import com.fasterxml.jackson.annotation.JsonInclude;

import java.util.Map;

@JsonInclude(JsonInclude.Include.NON_NULL)
public record SignalingEnvelope(
    String type,
    String roomCode,
    String fromUserId,
    String toUserId,
    Map<String, Object> payload
) {
}

