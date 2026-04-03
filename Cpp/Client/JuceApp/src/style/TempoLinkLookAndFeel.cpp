#include "tempolink/juce/style/TempoLinkLookAndFeel.h"
#include "tempolink/juce/style/UiStyle.h"

namespace tempolink::juceapp::style {

TempoLinkLookAndFeel::TempoLinkLookAndFeel() {
  ThemeManager::getInstance().addListener(this);
  themeChanged();
}

TempoLinkLookAndFeel::~TempoLinkLookAndFeel() {
  ThemeManager::getInstance().removeListener(this);
}

void TempoLinkLookAndFeel::themeChanged() {
  setColour(juce::ResizableWindow::backgroundColourId, LobbyBackground());
  setColour(juce::TextButton::buttonColourId, PrimaryBlue());
  setColour(juce::TextButton::textColourOffId, TextPrimary());
  setColour(juce::TextButton::textColourOnId, TextInverted());
  
  setColour(juce::Label::textColourId, TextPrimary());
  
  setColour(juce::ComboBox::backgroundColourId, PanelBackground());
  setColour(juce::ComboBox::textColourId, TextPrimary());
  setColour(juce::ComboBox::outlineColourId, PanelBorder());
  setColour(juce::ComboBox::arrowColourId, TextSecondary());
  
  setColour(juce::ToggleButton::textColourId, TextPrimary());
  setColour(juce::ToggleButton::tickColourId, AccentCyan());
  
  setColour(juce::Slider::thumbColourId, AccentCyan());
  setColour(juce::Slider::trackColourId, PanelBorder());
  setColour(juce::Slider::backgroundColourId, PanelBackground());
  setColour(juce::Slider::textBoxTextColourId, TextPrimary());
  setColour(juce::Slider::textBoxBackgroundColourId, CardBackground());
  setColour(juce::Slider::textBoxOutlineColourId, PanelBorder());

  setColour(juce::TextEditor::backgroundColourId, PanelBackground());
  setColour(juce::TextEditor::textColourId, TextPrimary());
  setColour(juce::TextEditor::outlineColourId, PanelBorder());
  setColour(juce::TextEditor::focusedOutlineColourId, PrimaryBlue());
  setColour(juce::TextEditor::highlightColourId, PrimaryBlue().withAlpha(0.3f));
  setColour(juce::TextEditor::highlightedTextColourId, TextPrimary());

  setColour(juce::ListBox::backgroundColourId, LobbyBackground());
  setColour(juce::TableHeaderComponent::backgroundColourId, PanelBackground());
  setColour(juce::TableHeaderComponent::textColourId, TextSecondary());

  setColour(juce::ScrollBar::backgroundColourId, juce::Colours::transparentBlack);
  setColour(juce::ScrollBar::thumbColourId, PanelBorder());
}

void TempoLinkLookAndFeel::drawButtonBackground(juce::Graphics& g,
                                                juce::Button& button,
                                                const juce::Colour& background_colour,
                                                bool should_draw_button_as_highlighted,
                                                bool should_draw_button_as_down) {
  auto corner_size = 10.0f;
  auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);

  auto base_colour = background_colour;
  if (should_draw_button_as_down) base_colour = base_colour.darker(0.2f);
  else if (should_draw_button_as_highlighted) base_colour = base_colour.brighter(0.1f);

  g.setColour(base_colour);
  g.fillRoundedRectangle(bounds, corner_size);

  if (should_draw_button_as_highlighted && !should_draw_button_as_down) {
    g.setColour(base_colour.withAlpha(0.2f));
    g.drawRoundedRectangle(bounds, corner_size, 2.0f);
  }
}

void TempoLinkLookAndFeel::drawLinearSlider(juce::Graphics& g,
                                           int x, int y, int width, int height,
                                           float slider_pos,
                                           float min_slider_pos,
                                           float max_slider_pos,
                                           const juce::Slider::SliderStyle style,
                                           juce::Slider& slider) {
  auto is_vertical = slider.isHorizontal() ? false : true;
  auto track_width = 4.0f;

  g.setColour(PanelBorder());
  if (is_vertical) {
    g.fillRoundedRectangle(x + width * 0.5f - track_width * 0.5f, y, track_width, height, track_width * 0.5f);
  } else {
    g.fillRoundedRectangle(x, y + height * 0.5f - track_width * 0.5f, width, track_width, track_width * 0.5f);
  }

  auto thumb_size = 14.0f;
  g.setColour(AccentCyan());
  
  if (is_vertical) {
    g.fillEllipse(x + width * 0.5f - thumb_size * 0.5f, slider_pos - thumb_size * 0.5f, thumb_size, thumb_size);
    // Subtle glow
    g.setColour(AccentCyan().withAlpha(0.3f));
    g.drawEllipse(x + width * 0.5f - thumb_size * 0.5f - 2.0f, slider_pos - thumb_size * 0.5f - 2.0f, thumb_size + 4.0f, thumb_size + 4.0f, 1.5f);
  } else {
    g.fillEllipse(slider_pos - thumb_size * 0.5f, y + height * 0.5f - thumb_size * 0.5f, thumb_size, thumb_size);
    // Subtle glow
    g.setColour(AccentCyan().withAlpha(0.3f));
    g.drawEllipse(slider_pos - thumb_size * 0.5f - 2.0f, y + height * 0.5f - thumb_size * 0.5f - 2.0f, thumb_size + 4.0f, thumb_size + 4.0f, 1.5f);
  }
}

void TempoLinkLookAndFeel::drawScrollbar(juce::Graphics& g,
                                        juce::ScrollBar& scrollbar,
                                        int x, int y, int width, int height,
                                        bool is_scrollbar_vertical,
                                        int thumb_position,
                                        int thumb_size,
                                        bool is_mouse_over,
                                        bool is_mouse_down) {
  g.setColour(PanelBackground());
  g.fillAll();

  auto thumb_colour = PanelBorder().withAlpha(is_mouse_over ? 0.8f : 0.4f);
  g.setColour(thumb_colour);

  auto indent = 4.0f;
  if (is_scrollbar_vertical) {
    g.fillRoundedRectangle(x + indent, thumb_position + indent, width - indent * 2.0f, thumb_size - indent * 2.0f, (width - indent * 2.0f) * 0.5f);
  } else {
    g.fillRoundedRectangle(thumb_position + indent, y + indent, thumb_size - indent * 2.0f, height - indent * 2.0f, (height - indent * 2.0f) * 0.5f);
  }
}

juce::Font TempoLinkLookAndFeel::getLabelFont(juce::Label& label) {
  return juce::Font("Inter", 14.0f, juce::Font::plain);
}

}  // namespace tempolink::juceapp::style
