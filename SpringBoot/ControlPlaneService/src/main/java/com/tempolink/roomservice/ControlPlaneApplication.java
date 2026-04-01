package com.tempolink.roomservice;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.EnableAutoConfiguration;
import org.springframework.boot.autoconfigure.SpringBootApplication;

@SpringBootApplication
@EnableAutoConfiguration
public class ControlPlaneApplication {
  public static void main(String[] args) {
    SpringApplication.run(ControlPlaneApplication.class, args);
  }
}
