package com.tempolink.roomservice.api;

import com.tempolink.roomservice.dto.UserSummaryResponse;
import com.tempolink.roomservice.service.UserProfileService;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

import java.util.List;
import java.util.Map;

@RestController
@RequestMapping("/api/users")
public class UsersController {
  private final UserProfileService userProfileService;

  public UsersController(UserProfileService userProfileService) {
    this.userProfileService = userProfileService;
  }

  @GetMapping
  public ResponseEntity<Map<String, List<UserSummaryResponse>>> list(
      @RequestParam(name = "q", required = false) String query) {
    return ResponseEntity.ok(Map.of("users", userProfileService.listUsers(query)));
  }
}
