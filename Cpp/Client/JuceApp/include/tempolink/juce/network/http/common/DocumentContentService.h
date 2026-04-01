#pragma once

#include <functional>
#include <string>

#include <juce_core/juce_core.h>

class DocumentContentService {
 public:
  using Callback = std::function<void(bool, juce::String, juce::String)>;

  void loadDocumentAsync(const std::string& relative_path, Callback callback) const;
  void loadApiOrDocumentAsync(const std::string& base_url,
                              const std::string& api_path,
                              const std::string& fallback_relative_path,
                              Callback callback) const;
};
