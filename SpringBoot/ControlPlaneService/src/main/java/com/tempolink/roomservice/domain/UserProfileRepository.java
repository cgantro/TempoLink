package com.tempolink.roomservice.domain;

import org.springframework.data.jpa.repository.JpaRepository;

import java.util.List;
import java.util.Optional;

public interface UserProfileRepository extends JpaRepository<UserProfile, Long> {
  Optional<UserProfile> findByProviderAndProviderUserId(String provider, String providerUserId);

  Optional<UserProfile> findByUserId(String userId);

  List<UserProfile> findTop200ByOrderByUpdatedAtDesc();
}
