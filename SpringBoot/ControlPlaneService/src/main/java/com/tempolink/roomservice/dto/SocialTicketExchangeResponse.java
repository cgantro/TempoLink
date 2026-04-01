package com.tempolink.roomservice.dto;

import java.time.Instant;

public record SocialTicketExchangeResponse(
    String sessionToken,
    Instant sessionExpiresAt,
    String userId,
    String provider,
    String providerUserId,
    String email,
    String displayName,
    String avatarUrl
) {
}
