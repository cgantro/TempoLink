package com.tempolink.roomservice.dto;

import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.Size;

public record UpdateProfileRequest(
    @NotBlank String userId,
    @Size(max = 80) String displayName,
    @Size(max = 512) String bio
) {
}
