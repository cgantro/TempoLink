package com.tempolink.roomservice.config;

import org.springframework.boot.context.properties.ConfigurationProperties;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.List;

@Component
@ConfigurationProperties(prefix = "tempolink.network")
public class NetworkProperties {
  private List<String> stunUrls = new ArrayList<>();
  private boolean turnEnabled = false;
  private List<String> turnUrls = new ArrayList<>();
  private String turnUsername = "";
  private String turnPassword = "";
  private long credentialTtlSeconds = 600;

  public List<String> getStunUrls() {
    return stunUrls;
  }

  public void setStunUrls(List<String> stunUrls) {
    this.stunUrls = stunUrls;
  }

  public boolean isTurnEnabled() {
    return turnEnabled;
  }

  public void setTurnEnabled(boolean turnEnabled) {
    this.turnEnabled = turnEnabled;
  }

  public List<String> getTurnUrls() {
    return turnUrls;
  }

  public void setTurnUrls(List<String> turnUrls) {
    this.turnUrls = turnUrls;
  }

  public String getTurnUsername() {
    return turnUsername;
  }

  public void setTurnUsername(String turnUsername) {
    this.turnUsername = turnUsername;
  }

  public String getTurnPassword() {
    return turnPassword;
  }

  public void setTurnPassword(String turnPassword) {
    this.turnPassword = turnPassword;
  }

  public long getCredentialTtlSeconds() {
    return credentialTtlSeconds;
  }

  public void setCredentialTtlSeconds(long credentialTtlSeconds) {
    this.credentialTtlSeconds = credentialTtlSeconds;
  }
}

