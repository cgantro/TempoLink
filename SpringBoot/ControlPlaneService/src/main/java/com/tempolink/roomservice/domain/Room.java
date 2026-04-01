package com.tempolink.roomservice.domain;

import jakarta.persistence.CollectionTable;
import jakarta.persistence.Column;
import jakarta.persistence.ElementCollection;
import jakarta.persistence.Entity;
import jakarta.persistence.FetchType;
import jakarta.persistence.GeneratedValue;
import jakarta.persistence.GenerationType;
import jakarta.persistence.Id;
import jakarta.persistence.JoinColumn;
import jakarta.persistence.PrePersist;
import jakarta.persistence.Table;
import jakarta.persistence.UniqueConstraint;

import java.time.Instant;
import java.util.HashSet;
import java.util.Set;

@Entity
@Table(name = "rooms")
public class Room {
  @Id
  @GeneratedValue(strategy = GenerationType.IDENTITY)
  private Long id;

  @Column(name = "room_code", nullable = false, unique = true, updatable = false, length = 16)
  private String roomCode;

  @Column(name = "host_user_id", nullable = false, length = 100)
  private String hostUserId;

  @Column(name = "name", nullable = false, length = 120)
  private String name;

  @Column(name = "description", nullable = false, length = 512)
  private String description;

  @ElementCollection(fetch = FetchType.EAGER)
  @CollectionTable(
      name = "room_tags",
      joinColumns = @JoinColumn(name = "room_id"),
      uniqueConstraints = @UniqueConstraint(name = "uk_room_tag", columnNames = {"room_id", "tag"}))
  @Column(name = "tag", nullable = false, length = 50)
  private Set<String> tags = new HashSet<>();

  @Column(name = "is_public", nullable = false)
  private boolean isPublic = true;

  @Column(name = "has_password", nullable = false)
  private boolean hasPassword = false;

  @Column(name = "password_value", length = 120)
  private String passwordValue;

  @Column(name = "max_participants", nullable = false)
  private int maxParticipants;

  @Column(name = "relay_host", nullable = false, length = 255)
  private String relayHost;

  @Column(name = "relay_port", nullable = false)
  private int relayPort;

  @Column(name = "created_at", nullable = false, updatable = false)
  private Instant createdAt;

  @ElementCollection(fetch = FetchType.EAGER)
  @CollectionTable(
      name = "room_participants",
      joinColumns = @JoinColumn(name = "room_id"),
      uniqueConstraints = @UniqueConstraint(name = "uk_room_participant", columnNames = {"room_id", "user_id"}))
  @Column(name = "user_id", nullable = false, length = 100)
  private Set<String> participants = new HashSet<>();

  protected Room() {
  }

  public Room(String roomCode, String hostUserId, String name, String description, Set<String> tags,
              boolean isPublic, boolean hasPassword, String passwordValue,
              int maxParticipants, String relayHost, int relayPort) {
    this.roomCode = roomCode;
    this.hostUserId = hostUserId;
    this.name = name;
    this.description = description;
    if (tags != null) {
      this.tags.addAll(tags);
    }
    this.isPublic = isPublic;
    this.hasPassword = hasPassword;
    this.passwordValue = passwordValue;
    this.maxParticipants = maxParticipants;
    this.relayHost = relayHost;
    this.relayPort = relayPort;
    this.createdAt = Instant.now();
  }

  @PrePersist
  void onCreate() {
    if (createdAt == null) {
      createdAt = Instant.now();
    }
  }

  public String roomCode() {
    return roomCode;
  }

  public String hostUserId() {
    return hostUserId;
  }

  public String name() {
    return name;
  }

  public String description() {
    return description;
  }

  public Set<String> tags() {
    return tags;
  }

  public boolean isPublic() {
    return isPublic;
  }

  public boolean hasPassword() {
    return hasPassword;
  }

  public String passwordValue() {
    return passwordValue;
  }

  public int maxParticipants() {
    return maxParticipants;
  }

  public String relayHost() {
    return relayHost;
  }

  public int relayPort() {
    return relayPort;
  }

  public Instant createdAt() {
    return createdAt;
  }

  public Set<String> participants() {
    return participants;
  }

  public void updateMetadata(String name, String description, Set<String> tags,
                             boolean isPublic, boolean hasPassword, String passwordValue,
                             int maxParticipants) {
    this.name = name;
    this.description = description;
    this.tags.clear();
    if (tags != null) {
      this.tags.addAll(tags);
    }
    this.isPublic = isPublic;
    this.hasPassword = hasPassword;
    this.passwordValue = passwordValue;
    this.maxParticipants = maxParticipants;
  }
}
