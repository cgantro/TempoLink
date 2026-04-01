package com.tempolink.roomservice.config;

import org.springframework.boot.context.properties.ConfigurationProperties;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.List;

@Component
@ConfigurationProperties(prefix = "tempolink.auth")
public class AuthProperties {
  private String defaultSuccessRedirect = "http://127.0.0.1:8080/api/auth/social/result";
  private long ticketTtlSeconds = 300;
  private long sessionTtlSeconds = 86400;
  private List<String> allowedRedirectPrefixes = new ArrayList<>(List.of(
      "tempolink://auth/callback",
      "http://127.0.0.1",
      "http://localhost"
  ));

  public String getDefaultSuccessRedirect() {
    return defaultSuccessRedirect;
  }

  public void setDefaultSuccessRedirect(String defaultSuccessRedirect) {
    this.defaultSuccessRedirect = defaultSuccessRedirect;
  }

  public long getTicketTtlSeconds() {
    return ticketTtlSeconds;
  }

  public void setTicketTtlSeconds(long ticketTtlSeconds) {
    this.ticketTtlSeconds = ticketTtlSeconds;
  }

  public long getSessionTtlSeconds() {
    return sessionTtlSeconds;
  }

  public void setSessionTtlSeconds(long sessionTtlSeconds) {
    this.sessionTtlSeconds = sessionTtlSeconds;
  }

  public List<String> getAllowedRedirectPrefixes() {
    return allowedRedirectPrefixes;
  }

  public void setAllowedRedirectPrefixes(List<String> allowedRedirectPrefixes) {
    this.allowedRedirectPrefixes = allowedRedirectPrefixes;
  }
}

