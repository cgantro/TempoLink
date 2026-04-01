package com.tempolink.roomservice.auth;

public record SocialUserProfile(
    String userId,
    String provider,
    String providerUserId,
    String email,
    String displayName,
    String avatarUrl
) {
}
