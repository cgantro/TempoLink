package com.tempolink.roomservice.security;

import com.tempolink.roomservice.auth.SocialUserProfile;

import java.util.Map;

public final class OAuthUserProfileMapper {
  private OAuthUserProfileMapper() {
  }

  public static SocialUserProfile map(String provider, Map<String, Object> attributes) {
    final String normalizedProvider = provider == null ? "unknown" : provider.toLowerCase();
    final String providerUserId = pick(attributes, "sub", "id", "user_id", "uid");
    final String email = pick(attributes, "email");
    final String displayName = pick(attributes, "name", "displayName", "nickname", "email");
    final String avatarUrl = pick(attributes, "picture", "avatar_url", "profile_image");

    return new SocialUserProfile(
        "",
        normalizedProvider,
        providerUserId.isBlank() ? "unknown" : providerUserId,
        email,
        displayName.isBlank() ? normalizedProvider + "-user" : displayName,
        avatarUrl
    );
  }

  private static String pick(Map<String, Object> attributes, String... keys) {
    if (attributes == null || attributes.isEmpty()) {
      return "";
    }
    for (String key : keys) {
      final Object value = attributes.get(key);
      if (value != null) {
        final String text = String.valueOf(value).trim();
        if (!text.isBlank()) {
          return text;
        }
      }
    }
    return "";
  }
}
