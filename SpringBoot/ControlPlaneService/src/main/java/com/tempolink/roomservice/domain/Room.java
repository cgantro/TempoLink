package com.tempolink.roomservice.domain;

import java.time.Instant;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;

public class Room {
  private final String roomCode;
  private final String hostUserId;
  private final int maxParticipants;
  private final String relayHost;
  private final int relayPort;
  private final Instant createdAt;
  private final Set<String> participants = ConcurrentHashMap.newKeySet();

  public Room(String roomCode, String hostUserId, int maxParticipants, String relayHost, int relayPort) {
    this.roomCode = roomCode;
    this.hostUserId = hostUserId;
    this.maxParticipants = maxParticipants;
    this.relayHost = relayHost;
    this.relayPort = relayPort;
    this.createdAt = Instant.now();
    this.participants.add(hostUserId);
  }

  public String roomCode() {
    return roomCode;
  }

  public String hostUserId() {
    return hostUserId;
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
}

