package com.tempolink.roomservice.dto;

import jakarta.validation.constraints.NotBlank;

public record DeleteRoomRequest(@NotBlank String hostUserId) {
}

