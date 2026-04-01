package com.tempolink.roomservice.api;

import com.tempolink.roomservice.dto.ProfileResponse;
import com.tempolink.roomservice.dto.UpdateProfileRequest;
import com.tempolink.roomservice.service.UserProfileService;
import jakarta.validation.Valid;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

@RestController
@RequestMapping("/api/profile")
public class ProfileController {
  private final UserProfileService userProfileService;

  public ProfileController(UserProfileService userProfileService) {
    this.userProfileService = userProfileService;
  }

  @GetMapping("/me")
  public ResponseEntity<ProfileResponse> me(@RequestParam("userId") String userId) {
    return ResponseEntity.ok(userProfileService.getProfile(userId));
  }

  @PutMapping("/me")
  public ResponseEntity<ProfileResponse> update(@Valid @RequestBody UpdateProfileRequest request) {
    return ResponseEntity.ok(
        userProfileService.updateProfile(request.userId(), request.displayName(), request.bio()));
  }
}
