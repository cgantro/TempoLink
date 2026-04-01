#pragma once

#include <functional>
#include <string>

#include <juce_core/juce_core.h>

struct UserProfileModel {
  juce::String user_id;
  juce::String display_name;
  juce::String bio;
  juce::String email;
  juce::String avatar_url;
  juce::String provider;
};

class ProfileApiClient {
 public:
  using FetchCallback = std::function<void(bool, UserProfileModel, juce::String)>;
  using UpdateCallback = std::function<void(bool, UserProfileModel, juce::String)>;

  explicit ProfileApiClient(std::string base_url);

  void fetchProfileAsync(const std::string& user_id, FetchCallback callback) const;
  void updateProfileAsync(const std::string& user_id,
                          const std::string& display_name,
                          const std::string& bio,
                          UpdateCallback callback) const;

 private:
  std::string base_url_;
};
