package com.tempolink.roomservice.dto;

import jakarta.validation.constraints.NotBlank;

public record JoinRoomRequest(@NotBlank String userId) {
}

