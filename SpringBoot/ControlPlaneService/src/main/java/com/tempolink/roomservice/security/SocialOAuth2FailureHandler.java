package com.tempolink.roomservice.security;

import com.tempolink.roomservice.config.AuthProperties;
import jakarta.servlet.ServletException;
import jakarta.servlet.http.HttpServletRequest;
import jakarta.servlet.http.HttpServletResponse;
import org.springframework.security.core.AuthenticationException;
import org.springframework.security.web.authentication.SimpleUrlAuthenticationFailureHandler;
import org.springframework.stereotype.Component;
import org.springframework.web.util.UriComponentsBuilder;

import java.io.IOException;

@Component
public class SocialOAuth2FailureHandler extends SimpleUrlAuthenticationFailureHandler {
  private final OAuthRedirectCookieService redirectCookieService;
  private final AuthProperties authProperties;

  public SocialOAuth2FailureHandler(
      OAuthRedirectCookieService redirectCookieService,
      AuthProperties authProperties) {
    this.redirectCookieService = redirectCookieService;
    this.authProperties = authProperties;
  }

  @Override
  public void onAuthenticationFailure(HttpServletRequest request,
                                      HttpServletResponse response,
                                      AuthenticationException exception) throws IOException, ServletException {
    final String redirectUri = redirectCookieService.readRedirectUri(request)
        .filter(uri -> !uri.isBlank())
        .filter(uri -> OAuthRedirectUriPolicy.isAllowed(uri, authProperties))
        .orElse(authProperties.getDefaultSuccessRedirect());
    redirectCookieService.clear(response);

    final String targetUri = UriComponentsBuilder.fromUriString(redirectUri)
        .queryParam("error", "oauth_failed")
        .queryParam("message", exception.getMessage())
        .build(true)
        .toUriString();
    getRedirectStrategy().sendRedirect(request, response, targetUri);
  }
}
