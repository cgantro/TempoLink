#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class PanelCard final : public juce::Component {
 public:
  PanelCard();

  void setTitle(const juce::String& title);
  void setContent(juce::Component& content);

  void resized() override;
  void paint(juce::Graphics& g) override;

 private:
  juce::Label title_label_;
  juce::Component* content_ = nullptr;
};
