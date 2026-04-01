package com.tempolink.roomservice.dto;

public record UserSummaryResponse(
    String userId,
    String displayName,
    String bio
) {
}
