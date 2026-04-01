package com.tempolink.roomservice.dto;

import jakarta.validation.constraints.Max;
import jakarta.validation.constraints.Min;
import jakarta.validation.constraints.NotBlank;

import java.util.List;

public record UpdateRoomRequest(
    @NotBlank String hostUserId,
    String name,
    String description,
    List<String> tags,
    Boolean isPublic,
    Boolean hasPassword,
    String password,
    @Min(2) @Max(6) Integer maxParticipants
) {
}

