package com.tempolink.roomservice.domain;

import jakarta.persistence.Column;
import jakarta.persistence.Entity;
import jakarta.persistence.GeneratedValue;
import jakarta.persistence.GenerationType;
import jakarta.persistence.Id;
import jakarta.persistence.PrePersist;
import jakarta.persistence.PreUpdate;
import jakarta.persistence.Table;
import jakarta.persistence.UniqueConstraint;

import java.time.Instant;

@Entity
@Table(
    name = "user_profiles",
    uniqueConstraints = @UniqueConstraint(
        name = "uk_user_profile_provider",
        columnNames = {"provider", "provider_user_id"}))
public class UserProfile {
  @Id
  @GeneratedValue(strategy = GenerationType.IDENTITY)
  private Long id;

  @Column(name = "user_id", nullable = false, unique = true, updatable = false, length = 100)
  private String userId;

  @Column(name = "provider", nullable = false, length = 32)
  private String provider;

  @Column(name = "provider_user_id", nullable = false, length = 128)
  private String providerUserId;

  @Column(name = "email", length = 255)
  private String email;

  @Column(name = "display_name", nullable = false, length = 80)
  private String displayName;

  @Column(name = "bio", nullable = false, length = 512)
  private String bio = "";

  @Column(name = "avatar_url", length = 512)
  private String avatarUrl;

  @Column(name = "created_at", nullable = false, updatable = false)
  private Instant createdAt;

  @Column(name = "updated_at", nullable = false)
  private Instant updatedAt;

  protected UserProfile() {
  }

  public UserProfile(String userId, String provider, String providerUserId,
                     String email, String displayName, String avatarUrl) {
    this.userId = userId;
    this.provider = provider;
    this.providerUserId = providerUserId;
    this.email = email;
    this.displayName = displayName;
    this.avatarUrl = avatarUrl;
    this.bio = "";
    this.createdAt = Instant.now();
    this.updatedAt = this.createdAt;
  }

  @PrePersist
  void onCreate() {
    final Instant now = Instant.now();
    if (createdAt == null) {
      createdAt = now;
    }
    if (updatedAt == null) {
      updatedAt = now;
    }
  }

  @PreUpdate
  void onUpdate() {
    updatedAt = Instant.now();
  }

  public String userId() {
    return userId;
  }

  public String provider() {
    return provider;
  }

  public String providerUserId() {
    return providerUserId;
  }

  public String email() {
    return email;
  }

  public String displayName() {
    return displayName;
  }

  public String bio() {
    return bio;
  }

  public String avatarUrl() {
    return avatarUrl;
  }

  public Instant createdAt() {
    return createdAt;
  }

  public Instant updatedAt() {
    return updatedAt;
  }

  public void updateFromSocial(String email, String fallbackDisplayName, String avatarUrl) {
    this.email = email;
    this.avatarUrl = avatarUrl;
    if ((this.displayName == null || this.displayName.isBlank()) &&
        fallbackDisplayName != null && !fallbackDisplayName.isBlank()) {
      this.displayName = fallbackDisplayName.trim();
    }
  }

  public void updateProfile(String displayName, String bio) {
    if (displayName != null && !displayName.isBlank()) {
      this.displayName = displayName.trim();
    }
    this.bio = bio == null ? "" : bio.trim();
  }
}
