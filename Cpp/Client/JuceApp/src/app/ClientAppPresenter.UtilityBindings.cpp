#include "tempolink/juce/app/ClientAppPresenter.h"

#include <utility>

void ClientAppPresenter::wireUtilityUiEvents() {
  using ScreenMode = tempolink::juceapp::app::ScreenMode;

  profile_view_.setOnBack([this] { screen_navigator_.SwitchTo(ScreenMode::Lobby); });

  users_view_.setOnBack([this] { screen_navigator_.SwitchTo(ScreenMode::Lobby); });
  users_view_.setOnRefresh(
      [this](std::string query) { content_controller_.refreshUsers(query); });

  news_view_.setOnBack([this] { screen_navigator_.SwitchTo(ScreenMode::Lobby); });
  news_view_.setOnRefresh([this] { content_controller_.refreshNews(); });

  manual_view_.setOnBack([this] { screen_navigator_.SwitchTo(ScreenMode::Lobby); });
  manual_view_.setOnReload([this] { content_controller_.refreshManual(); });

  qna_view_.setOnBack([this] { screen_navigator_.SwitchTo(ScreenMode::Lobby); });
  qna_view_.setOnReload([this] { content_controller_.refreshQna(); });

  settings_view_.setOnBack([this] { screen_navigator_.SwitchTo(ScreenMode::Lobby); });
  settings_view_.setOnInputDeviceChanged([this](std::string input_device) {
    preferred_input_device_ = std::move(input_device);
    if (session_.SetInputDevice(preferred_input_device_)) {
      settings_view_.setStatusText("Input device updated.");
    } else {
      settings_view_.setStatusText("Failed to change input device.");
    }
  });
  settings_view_.setOnOutputDeviceChanged([this](std::string output_device) {
    preferred_output_device_ = std::move(output_device);
    if (session_.SetOutputDevice(preferred_output_device_)) {
      settings_view_.setStatusText("Output device updated.");
    } else {
      settings_view_.setStatusText("Failed to change output device.");
    }
  });
  settings_view_.setOnApplyAudio([this](int sample_rate_hz, int buffer_samples) {
    content_controller_.applySettingsAudioFormat(sample_rate_hz, buffer_samples);
  });
  settings_view_.setOnTestInput([this] { content_controller_.runSettingsInputTest(); });
  settings_view_.setOnTestOutput([this] { content_controller_.runSettingsOutputTest(); });
  settings_view_.setOnSaveProfile([this](std::string display_name, std::string bio) {
    content_controller_.saveProfileSettings(display_name, bio);
  });
}
