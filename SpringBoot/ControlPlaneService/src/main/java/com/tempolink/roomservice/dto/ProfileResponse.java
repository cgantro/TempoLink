package com.tempolink.roomservice.dto;

public record ProfileResponse(
    String userId,
    String provider,
    String email,
    String displayName,
    String bio,
    String avatarUrl
) {
}
