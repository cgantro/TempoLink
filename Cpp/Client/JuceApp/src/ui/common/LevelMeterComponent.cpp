#include "tempolink/juce/ui/common/LevelMeterComponent.h"
#include <algorithm>
#include "tempolink/juce/style/UiStyle.h"

namespace tempolink::juceapp::ui::common {

LevelMeterComponent::LevelMeterComponent() {
  startTimerHz(60);
}

LevelMeterComponent::~LevelMeterComponent() {
  stopTimer();
}

void LevelMeterComponent::setLevel(float new_level) {
  current_level_ = std::max(current_level_, juce::jlimit(0.0f, 1.0f, new_level));
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
  // Smooth meter movement to reduce visible flicker:
  // fast attack, slower release.
  constexpr float kAttack = 0.45f;
  constexpr float kRelease = 0.12f;
  const float coeff = (current_level_ > falling_level_) ? kAttack : kRelease;
  falling_level_ += (current_level_ - falling_level_) * coeff;

  // Keep a gentle passive decay so the meter drops naturally
  // when new level updates stop arriving.
  current_level_ = std::max(0.0f, current_level_ - 0.015f);
  peak_level_ = std::max(0.0f, peak_level_ - 0.004f);

  repaint();
}

}  // namespace tempolink::juceapp::ui::common
