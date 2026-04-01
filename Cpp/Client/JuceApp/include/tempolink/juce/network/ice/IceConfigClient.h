#pragma once

#include <functional>
#include <string>

#include <juce_core/juce_core.h>

#include "tempolink/juce/network/ice/IceConfigTypes.h"

class IceConfigClient {
 public:
  using Callback = std::function<void(bool, IceConfigSnapshot, juce::String)>;

  explicit IceConfigClient(std::string base_url);

  void fetchIceConfigAsync(Callback callback) const;

 private:
  std::string base_url_;
};
