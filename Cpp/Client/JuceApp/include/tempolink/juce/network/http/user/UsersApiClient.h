#pragma once

#include <functional>
#include <string>
#include <vector>

#include <juce_core/juce_core.h>

struct UserSummary {
  std::string user_id;
  juce::String display_name;
  juce::String bio;
};

class UsersApiClient {
 public:
  using Callback = std::function<void(bool, std::vector<UserSummary>, juce::String)>;

  explicit UsersApiClient(std::string base_url);

  void fetchUsersAsync(const std::string& query, Callback callback) const;

 private:
  std::string base_url_;
};
