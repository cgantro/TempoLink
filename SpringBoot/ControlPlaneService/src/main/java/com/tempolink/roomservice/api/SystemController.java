package com.tempolink.roomservice.api;

import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import java.util.Map;

@RestController
@RequestMapping("/api/system")
public class SystemController {
  @GetMapping("/relay-endpoint")
  public Map<String, Object> relayEndpoint() {
    return Map.of(
        "host", "127.0.0.1",
        "port", 40000,
        "protocol", "udp"
    );
  }
}

