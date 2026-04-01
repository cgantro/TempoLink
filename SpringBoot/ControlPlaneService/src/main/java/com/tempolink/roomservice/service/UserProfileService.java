package com.tempolink.roomservice.service;

import com.tempolink.roomservice.auth.SocialUserProfile;
import com.tempolink.roomservice.domain.UserProfile;
import com.tempolink.roomservice.domain.UserProfileRepository;
import com.tempolink.roomservice.dto.ProfileResponse;
import com.tempolink.roomservice.dto.UserSummaryResponse;
import lombok.RequiredArgsConstructor;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.security.SecureRandom;
import java.util.Comparator;
import java.util.List;
import java.util.Locale;

@Service
@RequiredArgsConstructor
public class UserProfileService {
  private static final String USER_ID_CHARS = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";
  private static final int USER_ID_ATTEMPTS = 32;

  private final SecureRandom random = new SecureRandom();
  private final UserProfileRepository userProfileRepository;

  @Transactional
  public SocialUserProfile resolveSocialProfile(SocialUserProfile socialProfile) {
    final String provider = safeText(socialProfile.provider(), "google");
    final String providerUserId = safeText(socialProfile.providerUserId(), "unknown");
    final String displayName = safeText(
        socialProfile.displayName(), provider + "-user");

    final UserProfile profile = userProfileRepository
        .findByProviderAndProviderUserId(provider, providerUserId)
        .orElseGet(() -> userProfileRepository.save(new UserProfile(
            generateUserId(),
            provider,
            providerUserId,
            safeText(socialProfile.email(), ""),
            displayName,
            safeText(socialProfile.avatarUrl(), "")
        )));

    profile.updateFromSocial(
        safeText(socialProfile.email(), ""),
        displayName,
        safeText(socialProfile.avatarUrl(), ""));

    return new SocialUserProfile(
        profile.userId(),
        profile.provider(),
        profile.providerUserId(),
        safeText(profile.email(), ""),
        safeText(profile.displayName(), displayName),
        safeText(profile.avatarUrl(), "")
    );
  }

  @Transactional(readOnly = true)
  public ProfileResponse getProfile(String userId) {
    final UserProfile profile = userProfileRepository.findByUserId(userId)
        .orElseThrow(() -> new IllegalArgumentException("User not found: " + userId));
    return toProfile(profile);
  }

  @Transactional
  public ProfileResponse updateProfile(String userId, String displayName, String bio) {
    final UserProfile profile = userProfileRepository.findByUserId(userId)
        .orElseThrow(() -> new IllegalArgumentException("User not found: " + userId));
    profile.updateProfile(displayName, bio);
    return toProfile(profile);
  }

  @Transactional(readOnly = true)
  public List<UserSummaryResponse> listUsers(String query) {
    final String q = query == null ? "" : query.trim().toLowerCase(Locale.ROOT);
    return userProfileRepository.findTop200ByOrderByUpdatedAtDesc().stream()
        .filter(profile -> q.isEmpty()
            || safeText(profile.displayName(), "").toLowerCase(Locale.ROOT).contains(q)
            || safeText(profile.bio(), "").toLowerCase(Locale.ROOT).contains(q)
            || safeText(profile.userId(), "").toLowerCase(Locale.ROOT).contains(q))
        .sorted(Comparator.comparing(UserProfile::updatedAt).reversed())
        .map(profile -> new UserSummaryResponse(
            profile.userId(),
            safeText(profile.displayName(), "Unknown"),
            safeText(profile.bio(), "")))
        .toList();
  }

  private ProfileResponse toProfile(UserProfile profile) {
    return new ProfileResponse(
        profile.userId(),
        profile.provider(),
        safeText(profile.email(), ""),
        safeText(profile.displayName(), ""),
        safeText(profile.bio(), ""),
        safeText(profile.avatarUrl(), "")
    );
  }

  private String generateUserId() {
    for (int attempt = 0; attempt < USER_ID_ATTEMPTS; ++attempt) {
      final String candidate = "user-" + randomToken(8);
      if (userProfileRepository.findByUserId(candidate).isEmpty()) {
        return candidate;
      }
    }
    throw new IllegalStateException("Failed to generate unique userId");
  }

  private String randomToken(int length) {
    StringBuilder builder = new StringBuilder(length);
    for (int i = 0; i < length; ++i) {
      builder.append(USER_ID_CHARS.charAt(random.nextInt(USER_ID_CHARS.length())));
    }
    return builder.toString();
  }

  private static String safeText(String text, String fallback) {
    if (text == null || text.isBlank()) {
      return fallback;
    }
    return text.trim();
  }
}
