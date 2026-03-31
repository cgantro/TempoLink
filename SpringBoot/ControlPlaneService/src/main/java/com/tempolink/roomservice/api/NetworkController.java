package com.tempolink.roomservice.api;

import com.tempolink.roomservice.config.NetworkProperties;
import com.tempolink.roomservice.dto.IceConfigResponse;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import java.util.ArrayList;
import java.util.List;

@RestController
@RequestMapping("/api/network")
public class NetworkController {
  private final NetworkProperties networkProperties;

  public NetworkController(NetworkProperties networkProperties) {
    this.networkProperties = networkProperties;
  }

  @GetMapping("/ice")
  public ResponseEntity<IceConfigResponse> iceConfig() {
    List<IceConfigResponse.IceServerInfo> servers = new ArrayList<>();

    if (networkProperties.getStunUrls() != null && !networkProperties.getStunUrls().isEmpty()) {
      servers.add(new IceConfigResponse.IceServerInfo(networkProperties.getStunUrls(), null, null));
    }

    if (networkProperties.isTurnEnabled()
        && networkProperties.getTurnUrls() != null
        && !networkProperties.getTurnUrls().isEmpty()) {
      servers.add(new IceConfigResponse.IceServerInfo(
          networkProperties.getTurnUrls(),
          networkProperties.getTurnUsername(),
          networkProperties.getTurnPassword()
      ));
    }

    IceConfigResponse response =
        new IceConfigResponse(servers, networkProperties.getCredentialTtlSeconds());
    return ResponseEntity.ok(response);
  }
}

