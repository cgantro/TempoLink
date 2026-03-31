package com.tempolink.roomservice.dto;

import jakarta.validation.constraints.Max;
import jakarta.validation.constraints.Min;
import jakarta.validation.constraints.NotBlank;

public record CreateRoomRequest(
    @NotBlank String hostUserId,
    @Min(2) @Max(16) Integer maxParticipants,
    String relayHost,
    Integer relayPort
) {
}

