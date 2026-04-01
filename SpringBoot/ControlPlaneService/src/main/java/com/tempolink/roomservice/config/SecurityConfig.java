package com.tempolink.roomservice.config;

import com.tempolink.roomservice.security.SocialOAuth2FailureHandler;
import com.tempolink.roomservice.security.SocialOAuth2SuccessHandler;
import org.springframework.beans.factory.ObjectProvider;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.security.config.Customizer;
import org.springframework.security.config.annotation.web.builders.HttpSecurity;
import org.springframework.security.config.annotation.web.configurers.AbstractHttpConfigurer;
import org.springframework.security.oauth2.client.registration.ClientRegistrationRepository;
import org.springframework.security.web.SecurityFilterChain;

@Configuration
public class SecurityConfig {
  @Bean
  SecurityFilterChain securityFilterChain(
      HttpSecurity http,
      ObjectProvider<ClientRegistrationRepository> clientRegistrationRepositoryProvider,
      SocialOAuth2SuccessHandler successHandler,
      SocialOAuth2FailureHandler failureHandler) throws Exception {

    http
        .csrf(AbstractHttpConfigurer::disable)
        .headers(headers -> headers.frameOptions(frame -> frame.sameOrigin()))
        .authorizeHttpRequests(auth -> auth
            .requestMatchers("/h2-console/**").permitAll()
            .anyRequest().permitAll())
        .httpBasic(AbstractHttpConfigurer::disable)
        .formLogin(AbstractHttpConfigurer::disable);

    if (clientRegistrationRepositoryProvider.getIfAvailable() != null) {
      http.oauth2Login(oauth2 -> oauth2
          .successHandler(successHandler)
          .failureHandler(failureHandler));
    }

    http.logout(Customizer.withDefaults());
    return http.build();
  }
}

