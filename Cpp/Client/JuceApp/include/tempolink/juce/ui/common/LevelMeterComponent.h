#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace tempolink::juceapp::ui::common {

class LevelMeterComponent : public juce::Component, public juce::Timer {
 public:
  LevelMeterComponent();
  ~LevelMeterComponent() override;

  void setLevel(float new_level);
  void paint(juce::Graphics& g) override;
  void timerCallback() override;

 private:
  float current_level_ = 0.0f;
  float peak_level_ = 0.0f;
  float falling_level_ = 0.0f;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LevelMeterComponent)
};

}  // namespace tempolink::juceapp::ui::common
