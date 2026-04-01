#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "tempolink/juce/style/ThemeManager.h"

namespace tempolink::juceapp::style {

class TempoLinkLookAndFeel : public juce::LookAndFeel_V4,
                           public ThemeManager::Listener {
 public:
  TempoLinkLookAndFeel();
  ~TempoLinkLookAndFeel() override;

  void themeChanged() override;

  void drawButtonBackground(juce::Graphics& g,
                            juce::Button& button,
                            const juce::Colour& background_colour,
                            bool should_draw_button_as_highlighted,
                            bool should_draw_button_as_down) override;

  void drawLinearSlider(juce::Graphics& g,
                        int x, int y, int width, int height,
                        float slider_pos,
                        float min_slider_pos,
                        float max_slider_pos,
                        const juce::Slider::SliderStyle style,
                        juce::Slider& slider) override;

  void drawScrollbar(juce::Graphics& g,
                     juce::ScrollBar& scrollbar,
                     int x, int y, int width, int height,
                     bool is_scrollbar_vertical,
                     int thumb_position,
                     int thumb_size,
                     bool is_mouse_over,
                     bool is_mouse_down) override;

  juce::Font getLabelFont(juce::Label& label) override;
};

}  // namespace tempolink::juceapp::style
