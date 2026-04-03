#include "tempolink/juce/ui/screens/LoginView.h"

#include <algorithm>
#include <utility>

#include "tempolink/juce/style/UiStyle.h"

LoginView::LoginView() {
  title_label_.setText("TempoLink", juce::dontSendNotification);
  title_label_.setFont(juce::FontOptions(40.0F).withStyle("Bold"));
  title_label_.setJustificationType(juce::Justification::centred);
  addAndMakeVisible(title_label_);

  subtitle_label_.setText("Sign in to enter online jam rooms", juce::dontSendNotification);
  subtitle_label_.setFont(juce::FontOptions(16.0F));
  subtitle_label_.setJustificationType(juce::Justification::centred);
  addAndMakeVisible(subtitle_label_);

  status_label_.setText("Loading OAuth providers...", juce::dontSendNotification);
  status_label_.setFont(juce::FontOptions(14.0F));
  status_label_.setJustificationType(juce::Justification::centred);
  addAndMakeVisible(status_label_);

  google_login_button_.onClick = [this] {
    if (!busy_ && on_login_requested_) {
      on_login_requested_("google");
    }
  };
  addAndMakeVisible(google_login_button_);

  refresh_button_.onClick = [this] {
    if (!busy_ && on_refresh_providers_) {
      on_refresh_providers_();
    }
  };
  addAndMakeVisible(refresh_button_);
  
  updateTheme();
}
void LoginView::updateTheme() {
  title_label_.setColour(juce::Label::textColourId, tempolink::juceapp::style::TextPrimary());
  subtitle_label_.setColour(juce::Label::textColourId, tempolink::juceapp::style::TextSecondary());
  status_label_.setColour(juce::Label::textColourId, tempolink::juceapp::style::TextSecondary());
  
  google_login_button_.setColour(juce::TextButton::buttonColourId, tempolink::juceapp::style::PrimaryBlue());
  google_login_button_.setColour(juce::TextButton::textColourOffId, tempolink::juceapp::style::TextInverted());
  
  refresh_button_.setColour(juce::TextButton::buttonColourId, tempolink::juceapp::style::CardBackground());
  refresh_button_.setColour(juce::TextButton::textColourOffId, tempolink::juceapp::style::TextPrimary());
  
  repaint();
}

void LoginView::setOnLoginRequested(
    std::function<void(std::string)> on_login_requested) {
  on_login_requested_ = std::move(on_login_requested);
}

void LoginView::setOnRefreshProviders(std::function<void()> on_refresh_providers) {
  on_refresh_providers_ = std::move(on_refresh_providers);
}

void LoginView::setProviders(const std::vector<AuthProviderInfo>& providers) {
  google_enabled_ = false;
  for (const auto& provider : providers) {
    if (provider.id == "google") {
      google_enabled_ = provider.enabled;
    }
  }

  google_login_button_.setEnabled(!busy_ && google_enabled_);
}

void LoginView::setStatusText(const std::string& status_text) {
  status_label_.setText(juce::String(status_text), juce::dontSendNotification);
}

void LoginView::setBusy(bool busy) {
  busy_ = busy;
  refresh_button_.setEnabled(!busy_);
  google_login_button_.setEnabled(!busy_ && google_enabled_);
}

void LoginView::resized() {
  auto area = getLocalBounds().reduced(20);

  const int content_width = std::min(560, area.getWidth());
  const int content_height = std::min(320, area.getHeight());
  auto content = juce::Rectangle<int>(content_width, content_height)
                     .withCentre(area.getCentre());

  title_label_.setBounds(content.removeFromTop(70));
  subtitle_label_.setBounds(content.removeFromTop(30));
  content.removeFromTop(20);
  google_login_button_.setBounds(content.removeFromTop(42).reduced(80, 0));
  content.removeFromTop(14);
  refresh_button_.setBounds(content.removeFromTop(34).reduced(140, 0));
  content.removeFromTop(14);
  status_label_.setBounds(content.removeFromTop(60).reduced(20, 0));
}

void LoginView::paint(juce::Graphics& g) {
  g.fillAll(tempolink::juceapp::style::LobbyBackground());
}
