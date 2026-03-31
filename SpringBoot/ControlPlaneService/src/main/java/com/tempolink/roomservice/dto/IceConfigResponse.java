package com.tempolink.roomservice.dto;

import java.util.List;

public record IceConfigResponse(
    List<IceServerInfo> iceServers,
    long credentialTtlSeconds
) {
  public record IceServerInfo(
      List<String> urls,
      String username,
      String credential
  ) {
  }
}

