package com.tempolink.roomservice.security;

import jakarta.servlet.http.Cookie;
import jakarta.servlet.http.HttpServletRequest;
import jakarta.servlet.http.HttpServletResponse;
import org.springframework.stereotype.Component;

import java.net.URLDecoder;
import java.net.URLEncoder;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.Optional;

@Component
public class OAuthRedirectCookieService {
  private static final String COOKIE_NAME = "tl_oauth_redirect";
  private static final int COOKIE_MAX_AGE_SECONDS = 300;

  public void writeRedirectUri(HttpServletResponse response, String redirectUri) {
    final String encoded = URLEncoder.encode(redirectUri, StandardCharsets.UTF_8);
    Cookie cookie = new Cookie(COOKIE_NAME, encoded);
    cookie.setHttpOnly(true);
    cookie.setPath("/");
    cookie.setMaxAge(COOKIE_MAX_AGE_SECONDS);
    response.addCookie(cookie);
  }

  public Optional<String> readRedirectUri(HttpServletRequest request) {
    if (request.getCookies() == null) {
      return Optional.empty();
    }
    return Arrays.stream(request.getCookies())
        .filter(cookie -> COOKIE_NAME.equals(cookie.getName()))
        .findFirst()
        .map(Cookie::getValue)
        .map(value -> URLDecoder.decode(value, StandardCharsets.UTF_8));
  }

  public void clear(HttpServletResponse response) {
    Cookie cookie = new Cookie(COOKIE_NAME, "");
    cookie.setHttpOnly(true);
    cookie.setPath("/");
    cookie.setMaxAge(0);
    response.addCookie(cookie);
  }
}

