package com.tempolink.roomservice.config;

import com.tempolink.roomservice.signaling.SignalingWebSocketHandler;
import org.springframework.context.annotation.Configuration;
import org.springframework.web.socket.config.annotation.EnableWebSocket;
import org.springframework.web.socket.config.annotation.WebSocketConfigurer;
import org.springframework.web.socket.config.annotation.WebSocketHandlerRegistry;

@Configuration
@EnableWebSocket
public class SignalingWebSocketConfig implements WebSocketConfigurer {
  private final SignalingWebSocketHandler signalingWebSocketHandler;

  public SignalingWebSocketConfig(SignalingWebSocketHandler signalingWebSocketHandler) {
    this.signalingWebSocketHandler = signalingWebSocketHandler;
  }

  @Override
  public void registerWebSocketHandlers(WebSocketHandlerRegistry registry) {
    registry.addHandler(signalingWebSocketHandler, "/ws/signaling")
        .setAllowedOriginPatterns("*");
  }
}

