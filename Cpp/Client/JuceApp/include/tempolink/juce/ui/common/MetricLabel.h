#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "tempolink/juce/style/ThemeManager.h"

class MetricLabel final : public juce::Component,
                          public tempolink::juceapp::style::ThemeManager::Listener {
 public:
  MetricLabel();
  ~MetricLabel() override;

  void themeChanged() override;
  void updateTheme();

  void setTitle(const juce::String& title);
  void setValue(const juce::String& value);

  void resized() override;

 private:
  juce::Label title_label_;
  juce::Label value_label_;
};
