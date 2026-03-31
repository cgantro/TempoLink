package com.tempolink.roomservice.dto;

import jakarta.validation.constraints.NotBlank;

public record LeaveRoomRequest(@NotBlank String userId) {
}

