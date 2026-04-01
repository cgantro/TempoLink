#pragma once

#include <functional>
#include <string>
#include <vector>

#include <juce_core/juce_core.h>

struct NewsItem {
  juce::String title;
  juce::String body;
  juce::String published_at;
};

class NewsApiClient {
 public:
  using Callback = std::function<void(bool, std::vector<NewsItem>, juce::String)>;

  explicit NewsApiClient(std::string base_url);

  void fetchNewsAsync(Callback callback) const;

 private:
  std::string base_url_;
};
