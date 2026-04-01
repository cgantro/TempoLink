package com.tempolink.roomservice.api;

import com.tempolink.roomservice.config.AuthProperties;
import com.tempolink.roomservice.dto.AuthProviderInfoResponse;
import com.tempolink.roomservice.dto.AuthProvidersResponse;
import com.tempolink.roomservice.dto.SocialTicketExchangeResponse;
import com.tempolink.roomservice.security.OAuthRedirectCookieService;
import com.tempolink.roomservice.security.OAuthRedirectUriPolicy;
import com.tempolink.roomservice.service.SocialLoginTicketService;
import jakarta.servlet.http.HttpServletResponse;
import org.springframework.beans.factory.ObjectProvider;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.security.oauth2.client.registration.ClientRegistration;
import org.springframework.security.oauth2.client.registration.ClientRegistrationRepository;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Set;
import java.util.stream.Collectors;
import java.util.stream.Stream;

@RestController
@RequestMapping("/api/auth")
public class AuthController {
  private static final Set<String> SUPPORTED_PROVIDERS = Set.of("google");

  private final ObjectProvider<ClientRegistrationRepository> registrationRepositoryProvider;
  private final OAuthRedirectCookieService redirectCookieService;
  private final SocialLoginTicketService socialLoginTicketService;
  private final AuthProperties authProperties;

  public AuthController(
      ObjectProvider<ClientRegistrationRepository> registrationRepositoryProvider,
      OAuthRedirectCookieService redirectCookieService,
      SocialLoginTicketService socialLoginTicketService,
      AuthProperties authProperties) {
    this.registrationRepositoryProvider = registrationRepositoryProvider;
    this.redirectCookieService = redirectCookieService;
    this.socialLoginTicketService = socialLoginTicketService;
    this.authProperties = authProperties;
  }

  @GetMapping("/providers")
  public ResponseEntity<AuthProvidersResponse> providers() {
    final Set<String> enabledProviders = enabledProviderIds();
    final List<AuthProviderInfoResponse> providers = Stream.of("google")
        .map(id -> new AuthProviderInfoResponse(
            id,
            enabledProviders.contains(id),
            "/api/auth/social/" + id + "/start"
        ))
        .toList();
    return ResponseEntity.ok(new AuthProvidersResponse(providers));
  }

  @GetMapping("/social/{provider}/start")
  public void startSocialLogin(@PathVariable("provider") String provider,
                               @RequestParam(name = "redirectUri", required = false) String redirectUri,
                               HttpServletResponse response) throws IOException {
    // Clear stale redirect cookie before writing a new callback target.
    redirectCookieService.clear(response);

    final String normalizedProvider = normalizeProvider(provider);
    if (!SUPPORTED_PROVIDERS.contains(normalizedProvider)) {
      response.sendError(HttpStatus.BAD_REQUEST.value(), "Unsupported provider: " + provider);
      return;
    }
    if (!enabledProviderIds().contains(normalizedProvider)) {
      response.sendError(HttpStatus.BAD_REQUEST.value(),
          "Provider is not configured: " + normalizedProvider);
      return;
    }
    if (redirectUri != null && !redirectUri.isBlank()) {
      if (!OAuthRedirectUriPolicy.isAllowed(redirectUri, authProperties)) {
        response.sendError(HttpStatus.BAD_REQUEST.value(), "Redirect URI is not allowed");
        return;
      }
      redirectCookieService.writeRedirectUri(response, redirectUri);
    } else {
      // Avoid reusing a stale redirect URI from a previous OAuth attempt.
      redirectCookieService.clear(response);
    }
    response.sendRedirect("/oauth2/authorization/" + normalizedProvider);
  }

  @GetMapping("/social/ticket/{ticket}")
  public ResponseEntity<?> exchangeTicket(@PathVariable("ticket") String ticket) {
    return socialLoginTicketService.consume(ticket)
        .<ResponseEntity<?>>map(record -> ResponseEntity.ok(new SocialTicketExchangeResponse(
            record.sessionToken(),
            record.sessionExpiresAt(),
            record.userProfile().userId(),
            record.userProfile().provider(),
            record.userProfile().providerUserId(),
            record.userProfile().email(),
            record.userProfile().displayName(),
            record.userProfile().avatarUrl()
        )))
        .orElseGet(() -> ResponseEntity.status(HttpStatus.NOT_FOUND).body(Map.of(
            "error", "INVALID_OR_EXPIRED_TICKET",
            "message", "Ticket is invalid, consumed, or expired"
        )));
  }

  @GetMapping("/social/result")
  public ResponseEntity<?> result(@RequestParam(name = "ticket", required = false) String ticket,
                                  @RequestParam(name = "provider", required = false) String provider,
                                  @RequestParam(name = "error", required = false) String error,
                                  @RequestParam(name = "message", required = false) String message) {
    if (error != null && !error.isBlank()) {
      return ResponseEntity.status(HttpStatus.BAD_REQUEST).body(Map.of(
          "status", "failed",
          "error", error,
          "message", message == null ? "" : message
      ));
    }
    return ResponseEntity.ok(Map.of(
        "status", "ok",
        "provider", provider == null ? "" : provider,
        "ticket", ticket == null ? "" : ticket,
        "next", "Call GET /api/auth/social/ticket/{ticket} from client"
    ));
  }

  private Set<String> enabledProviderIds() {
    final ClientRegistrationRepository repository = registrationRepositoryProvider.getIfAvailable();
    if (repository == null) {
      return Set.of();
    }
    if (!(repository instanceof Iterable<?> iterable)) {
      return Set.of();
    }

    final List<Object> entries = new ArrayList<>();
    iterable.forEach(entries::add);
    return entries.stream()
        .filter(ClientRegistration.class::isInstance)
        .map(ClientRegistration.class::cast)
        .filter(this::isConfiguredRegistration)
        .map(ClientRegistration::getRegistrationId)
        .map(this::normalizeProvider)
        .collect(Collectors.toSet());
  }

  private boolean isConfiguredRegistration(ClientRegistration registration) {
    return registration != null
        && registration.getClientId() != null
        && !registration.getClientId().isBlank();
  }

  private String normalizeProvider(String provider) {
    return provider == null ? "" : provider.trim().toLowerCase(Locale.ROOT);
  }
}
