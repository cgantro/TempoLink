#include "tempolink/juce/ui/common/LevelMeterComponent.h"
#include "tempolink/juce/style/UiStyle.h"

namespace tempolink::juceapp::ui::common {

LevelMeterComponent::LevelMeterComponent() {
  startTimerHz(30);
}

LevelMeterComponent::~LevelMeterComponent() {
  stopTimer();
}

void LevelMeterComponent::setLevel(float new_level) {
  current_level_ = new_level;
  if (new_level > peak_level_) {
    peak_level_ = new_level;
  }
}

void LevelMeterComponent::paint(juce::Graphics& g) {
  auto bounds = getLocalBounds().toFloat();
  auto w = bounds.getWidth();
  auto h = bounds.getHeight();

  g.setColour(tempolink::juceapp::style::PanelBackground());
  g.fillRoundedRectangle(bounds, 2.0f);

  auto level_w = w * falling_level_;
  
  auto gradient = juce::ColourGradient::horizontal(
      tempolink::juceapp::style::ConnectedText(), 0, 
      juce::Colour::fromRGB(239, 68, 68), w  // Red at the right end
  );
  gradient.addColour(0.7f, tempolink::juceapp::style::SyncingText()); // Yellow at 70%
  
  g.setGradientFill(gradient);
  g.fillRoundedRectangle(0.0f, 0.0f, level_w, h, 2.0f);

  // Subtle glow (only in dark mode, or very light in light mode)
  if (falling_level_ > 0.1f) {
    auto alpha = tempolink::juceapp::style::ThemeManager::getInstance().isDark() ? 0.3f : 0.1f;
    g.setColour(tempolink::juceapp::style::AccentCyan().withAlpha(alpha));
    g.drawRoundedRectangle(bounds, 2.0f, 1.0f);
  }
}

void LevelMeterComponent::timerCallback() {
  if (falling_level_ < current_level_) {
    falling_level_ = current_level_;
  } else {
    falling_level_ = std::max(0.0f, falling_level_ - 0.05f);
  }
  
  peak_level_ = std::max(0.0f, peak_level_ - 0.01f);
  current_level_ = std::max(0.0f, current_level_ - 0.1f);
  
  repaint();
}

}  // namespace tempolink::juceapp::ui::common
