#include "tempolink/juce/app/ClientAppPresenter.h"

#include <utility>

void ClientAppPresenter::wireUtilityUiEvents() {
  using ScreenMode = tempolink::juceapp::app::ScreenMode;

  views_.profile_view.setOnBack([this] { screen_navigator_.SwitchTo(ScreenMode::Lobby); });

  views_.users_view.setOnBack([this] { screen_navigator_.SwitchTo(ScreenMode::Lobby); });
  views_.users_view.setOnRefresh(
      [this](std::string query) { content_controller_.refreshUsers(query); });

  views_.news_view.setOnBack([this] { screen_navigator_.SwitchTo(ScreenMode::Lobby); });
  views_.news_view.setOnRefresh([this] { content_controller_.refreshNews(); });

  views_.manual_view.setOnBack([this] { screen_navigator_.SwitchTo(ScreenMode::Lobby); });
  views_.manual_view.setOnReload([this] { content_controller_.refreshManual(); });

  views_.qna_view.setOnBack([this] { screen_navigator_.SwitchTo(ScreenMode::Lobby); });
  views_.qna_view.setOnReload([this] { content_controller_.refreshQna(); });

  views_.settings_view.setOnBack([this] { screen_navigator_.SwitchTo(ScreenMode::Lobby); });
  views_.settings_view.setOnInputDeviceChanged([this](std::string input_device) {
    status_context_.preferred_input_device = std::move(input_device);
    if (deps_.session.SetInputDevice(status_context_.preferred_input_device)) {
      views_.settings_view.setStatusText("Input device updated.");
    } else {
      views_.settings_view.setStatusText("Failed to change input device.");
    }
  });
  views_.settings_view.setOnOutputDeviceChanged([this](std::string output_device) {
    status_context_.preferred_output_device = std::move(output_device);
    if (deps_.session.SetOutputDevice(status_context_.preferred_output_device)) {
      views_.settings_view.setStatusText("Output device updated.");
    } else {
      views_.settings_view.setStatusText("Failed to change output device.");
    }
  });
  views_.settings_view.setOnApplyAudio([this](int sample_rate_hz, int buffer_samples) {
    content_controller_.applySettingsAudioFormat(sample_rate_hz, buffer_samples);
  });
  views_.settings_view.setOnTestInput([this] { content_controller_.runSettingsInputTest(); });
  views_.settings_view.setOnTestOutput([this] { content_controller_.runSettingsOutputTest(); });
  views_.settings_view.setOnSaveProfile([this](std::string display_name, std::string bio) {
    content_controller_.saveProfileSettings(display_name, bio);
  });
}
