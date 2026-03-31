#include "tempolink/client/EndpointManager.h"

#include <algorithm>
#include <utility>

namespace tempolink::client {

bool EndpointManager::AddOrUpdate(EndpointProfile profile) {
  if (profile.profile_name.empty()) {
    return false;
  }
  std::scoped_lock lock(mutex_);
  profiles_[profile.profile_name] = std::move(profile);
  if (active_profile_name_.empty()) {
    active_profile_name_ = profiles_.begin()->first;
  }
  return true;
}

bool EndpointManager::Remove(const std::string& profile_name) {
  std::scoped_lock lock(mutex_);
  const auto erased = profiles_.erase(profile_name);
  if (erased == 0) {
    return false;
  }
  if (active_profile_name_ == profile_name) {
    active_profile_name_.clear();
    if (!profiles_.empty()) {
      active_profile_name_ = profiles_.begin()->first;
    }
  }
  return true;
}

std::optional<EndpointProfile> EndpointManager::Find(
    const std::string& profile_name) const {
  std::scoped_lock lock(mutex_);
  const auto it = profiles_.find(profile_name);
  if (it == profiles_.end()) {
    return std::nullopt;
  }
  return it->second;
}

std::vector<std::string> EndpointManager::ListProfileNames() const {
  std::scoped_lock lock(mutex_);
  std::vector<std::string> names;
  names.reserve(profiles_.size());
  for (const auto& [name, _] : profiles_) {
    names.push_back(name);
  }
  std::sort(names.begin(), names.end());
  return names;
}

void EndpointManager::SetActiveProfile(const std::string& profile_name) {
  std::scoped_lock lock(mutex_);
  if (profiles_.contains(profile_name)) {
    active_profile_name_ = profile_name;
  }
}

std::optional<EndpointProfile> EndpointManager::ActiveProfile() const {
  std::scoped_lock lock(mutex_);
  if (active_profile_name_.empty()) {
    return std::nullopt;
  }
  const auto it = profiles_.find(active_profile_name_);
  if (it == profiles_.end()) {
    return std::nullopt;
  }
  return it->second;
}

}  // namespace tempolink::client

