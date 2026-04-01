package com.tempolink.roomservice.security;

import com.tempolink.roomservice.config.AuthProperties;

import java.net.URI;
import java.util.Set;

public final class OAuthRedirectUriPolicy {
  private static final Set<Integer> BLOCKED_HTTP_PORTS = Set.of(
      1, 7, 9, 11, 13, 15, 17, 19, 20, 21, 22, 23, 25, 37, 42, 43, 53, 69, 77, 79, 87,
      95, 101, 102, 103, 104, 109, 110, 111, 113, 115, 117, 119, 123, 135, 137, 139, 143,
      161, 179, 389, 427, 465, 512, 513, 514, 515, 526, 530, 531, 532, 540, 548, 554, 556,
      563, 587, 601, 636, 989, 990, 993, 995, 1719, 1720, 1723, 2049, 3659, 4045, 5060,
      5061, 6000, 6566, 6665, 6666, 6667, 6668, 6669, 6697, 10080);

  private OAuthRedirectUriPolicy() {}

  public static boolean isAllowed(String redirectUri, AuthProperties authProperties) {
    if (redirectUri == null || redirectUri.isBlank()) {
      return false;
    }

    final boolean prefixAllowed = authProperties.getAllowedRedirectPrefixes().stream()
        .filter(prefix -> prefix != null && !prefix.isBlank())
        .anyMatch(redirectUri::startsWith);
    if (!prefixAllowed) {
      return false;
    }

    final URI uri;
    try {
      uri = URI.create(redirectUri);
    } catch (Exception ignored) {
      return false;
    }

    final String scheme = uri.getScheme();
    if (scheme == null || scheme.isBlank()) {
      return false;
    }
    if ("tempolink".equalsIgnoreCase(scheme)) {
      return true;
    }

    if ("http".equalsIgnoreCase(scheme) || "https".equalsIgnoreCase(scheme)) {
      int port = uri.getPort();
      if (port <= 0) {
        port = "https".equalsIgnoreCase(scheme) ? 443 : 80;
      }
      if (port < 1024 || BLOCKED_HTTP_PORTS.contains(port)) {
        return false;
      }
      return true;
    }

    return false;
  }
}

