package com.tempolink.roomservice.security;

import com.tempolink.roomservice.config.AuthProperties;
import com.tempolink.roomservice.service.SocialLoginTicketService;
import com.tempolink.roomservice.service.UserProfileService;
import jakarta.servlet.ServletException;
import jakarta.servlet.http.HttpServletRequest;
import jakarta.servlet.http.HttpServletResponse;
import org.springframework.security.core.Authentication;
import org.springframework.security.oauth2.client.authentication.OAuth2AuthenticationToken;
import org.springframework.security.web.authentication.SimpleUrlAuthenticationSuccessHandler;
import org.springframework.stereotype.Component;
import org.springframework.web.util.UriComponentsBuilder;

import java.io.IOException;

@Component
public class SocialOAuth2SuccessHandler extends SimpleUrlAuthenticationSuccessHandler {
  private final SocialLoginTicketService socialLoginTicketService;
  private final UserProfileService userProfileService;
  private final OAuthRedirectCookieService redirectCookieService;
  private final AuthProperties authProperties;

  public SocialOAuth2SuccessHandler(
      SocialLoginTicketService socialLoginTicketService,
      UserProfileService userProfileService,
      OAuthRedirectCookieService redirectCookieService,
      AuthProperties authProperties) {
    this.socialLoginTicketService = socialLoginTicketService;
    this.userProfileService = userProfileService;
    this.redirectCookieService = redirectCookieService;
    this.authProperties = authProperties;
  }

  @Override
  public void onAuthenticationSuccess(HttpServletRequest request,
                                      HttpServletResponse response,
                                      Authentication authentication) throws IOException, ServletException {
    if (!(authentication instanceof OAuth2AuthenticationToken oauthToken)) {
      super.onAuthenticationSuccess(request, response, authentication);
      return;
    }

    final String provider = oauthToken.getAuthorizedClientRegistrationId();
    final var socialProfile =
        OAuthUserProfileMapper.map(provider, oauthToken.getPrincipal().getAttributes());
    final var resolvedProfile = userProfileService.resolveSocialProfile(socialProfile);
    final var ticketRecord = socialLoginTicketService.issue(resolvedProfile);

    final String redirectUri = redirectCookieService.readRedirectUri(request)
        .filter(uri -> !uri.isBlank())
        .filter(uri -> OAuthRedirectUriPolicy.isAllowed(uri, authProperties))
        .orElse(authProperties.getDefaultSuccessRedirect());
    redirectCookieService.clear(response);

    final String targetUri = UriComponentsBuilder.fromUriString(redirectUri)
        .queryParam("ticket", ticketRecord.ticket())
        .queryParam("provider", provider)
        .build(true)
        .toUriString();
    getRedirectStrategy().sendRedirect(request, response, targetUri);
  }
}
