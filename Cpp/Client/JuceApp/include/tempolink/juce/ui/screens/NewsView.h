#pragma once

#include <functional>
#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>
#include "tempolink/juce/style/ThemeableComponent.h"
#include "tempolink/juce/network/http/news/NewsApiClient.h"

class NewsView final : public tempolink::juceapp::style::ThemeableComponent {
 public:
  NewsView();
  void updateTheme() override;

  void setOnBack(std::function<void()> on_back);
  void setOnRefresh(std::function<void()> on_refresh);
  void setItems(const std::vector<NewsItem>& items);
  void setStatusText(const juce::String& status_text);

  void resized() override;
  void paint(juce::Graphics& g) override;

 private:
  std::function<void()> on_back_;
  std::function<void()> on_refresh_;

  juce::TextButton back_button_{"<- Rooms"};
  juce::Label title_label_;
  juce::Label status_label_;
  juce::TextButton refresh_button_{"Refresh"};
  juce::TextEditor news_text_;
};
