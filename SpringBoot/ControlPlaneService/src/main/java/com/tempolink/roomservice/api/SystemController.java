package com.tempolink.roomservice.api;

import com.tempolink.roomservice.config.RelayProperties;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import java.util.Map;

@RestController
@RequestMapping("/api/system")
public class SystemController {
  private final RelayProperties relayProperties;

  public SystemController(RelayProperties relayProperties) {
    this.relayProperties = relayProperties;
  }

  @GetMapping("/relay-endpoint")
  public Map<String, Object> relayEndpoint() {
    return Map.of(
        "host", relayProperties.getHost(),
        "port", relayProperties.getPort(),
        "protocol", "udp"
    );
  }
}
