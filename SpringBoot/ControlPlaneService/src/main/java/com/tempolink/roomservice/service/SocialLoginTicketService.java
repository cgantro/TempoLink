package com.tempolink.roomservice.service;

import com.tempolink.roomservice.auth.SocialUserProfile;
import com.tempolink.roomservice.config.AuthProperties;
import org.springframework.stereotype.Service;

import java.time.Instant;
import java.util.Optional;
import java.util.UUID;
import java.util.concurrent.ConcurrentHashMap;

@Service
public class SocialLoginTicketService {
  private final ConcurrentHashMap<String, TicketRecord> ticketStore = new ConcurrentHashMap<>();
  private final long ticketTtlSeconds;
  private final long sessionTtlSeconds;

  public SocialLoginTicketService(AuthProperties authProperties) {
    this.ticketTtlSeconds = Math.max(30, authProperties.getTicketTtlSeconds());
    this.sessionTtlSeconds = Math.max(300, authProperties.getSessionTtlSeconds());
  }

  public TicketRecord issue(SocialUserProfile userProfile) {
    cleanupExpired();
    final Instant now = Instant.now();
    final String ticket = UUID.randomUUID().toString();
    final String sessionToken = UUID.randomUUID().toString().replace("-", "");
    final TicketRecord record = new TicketRecord(
        ticket,
        sessionToken,
        now.plusSeconds(ticketTtlSeconds),
        now.plusSeconds(sessionTtlSeconds),
        userProfile
    );
    ticketStore.put(ticket, record);
    return record;
  }

  public Optional<TicketRecord> consume(String ticket) {
    cleanupExpired();
    if (ticket == null || ticket.isBlank()) {
      return Optional.empty();
    }
    final TicketRecord removed = ticketStore.remove(ticket);
    if (removed == null) {
      return Optional.empty();
    }
    if (removed.ticketExpiresAt().isBefore(Instant.now())) {
      return Optional.empty();
    }
    return Optional.of(removed);
  }

  private void cleanupExpired() {
    final Instant now = Instant.now();
    ticketStore.entrySet().removeIf(entry -> entry.getValue().ticketExpiresAt().isBefore(now));
  }

  public record TicketRecord(
      String ticket,
      String sessionToken,
      Instant ticketExpiresAt,
      Instant sessionExpiresAt,
      SocialUserProfile userProfile
  ) {
  }
}

