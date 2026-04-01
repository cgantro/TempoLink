#pragma once

#include <functional>

#include <juce_gui_basics/juce_gui_basics.h>
#include "tempolink/juce/style/ThemeableComponent.h"

class ManualView final : public tempolink::juceapp::style::ThemeableComponent {
 public:
  ManualView();
  void updateTheme() override;

  void setOnBack(std::function<void()> on_back);
  void setOnReload(std::function<void()> on_reload);
  void setStatusText(const juce::String& status_text);
  void setDocumentText(const juce::String& text);

  void resized() override;
  void paint(juce::Graphics& g) override;

 private:
  std::function<void()> on_back_;
  std::function<void()> on_reload_;

  juce::TextButton back_button_{"<- Rooms"};
  juce::TextButton reload_button_{"Reload"};
  juce::Label title_label_;
  juce::Label status_label_;
  juce::TextEditor doc_text_;
};
