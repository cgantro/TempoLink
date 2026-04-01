#include "tempolink/juce/app/content/ContentController.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cmath>
#include <span>
#include <thread>
#include <utility>
#include <vector>

#include <juce_core/juce_core.h>

#include "tempolink/audio/AudioInputFactory.h"
#include "tempolink/audio/AudioOutputFactory.h"
#include "tempolink/juce/constants/ApiPaths.h"

namespace {
std::atomic_bool g_input_test_running{false};
std::atomic_bool g_output_test_running{false};
}  // namespace

ContentController::ContentController(
    UsersView& users_view, NewsView& news_view, ProfileView& profile_view,
    ManualView& manual_view, QnaView& qna_view, SettingsView& settings_view,
    UsersApiClient& users_api, NewsApiClient& news_api,
    ProfileApiClient& profile_api, DocumentContentService& document_service,
    tempolink::client::ClientSession& session,
    std::shared_ptr<std::atomic_bool> alive_flag,
    const std::string& control_plane_base_url, std::string& current_user_id,
    std::string& current_display_name, std::string& current_bio)
    : users_view_(users_view),
      news_view_(news_view),
      profile_view_(profile_view),
      manual_view_(manual_view),
      qna_view_(qna_view),
      settings_view_(settings_view),
      users_api_(users_api),
      news_api_(news_api),
      profile_api_(profile_api),
      document_service_(document_service),
      session_(session),
      alive_flag_(std::move(alive_flag)),
      control_plane_base_url_(control_plane_base_url),
      current_user_id_(current_user_id),
      current_display_name_(current_display_name),
      current_bio_(current_bio) {}

void ContentController::refreshUsers(const std::string& query) {
  users_view_.setStatusText("Loading users...");
  auto alive = alive_flag_;
  users_api_.fetchUsersAsync(query, [this, alive](bool ok,
                                                  std::vector<UserSummary> users,
                                                  juce::String error_text) {
    if (!alive->load()) {
      return;
    }
    if (!ok) {
      users_view_.setStatusText("Users load failed: " + error_text);
      users_view_.setUsers({});
      return;
    }
    users_view_.setUsers(users);
    users_view_.setStatusText("Users: " +
                              juce::String(static_cast<int>(users.size())));
  });
}

void ContentController::refreshNews() {
  news_view_.setStatusText("Loading notices...");
  auto alive = alive_flag_;
  news_api_.fetchNewsAsync([this, alive](bool ok, std::vector<NewsItem> items,
                                         juce::String error_text) {
    if (!alive->load()) {
      return;
    }
    if (!ok) {
      news_view_.setStatusText("News load failed: " + error_text);
      news_view_.setItems({});
      return;
    }
    news_view_.setItems(items);
    news_view_.setStatusText("Notices: " +
                             juce::String(static_cast<int>(items.size())));
  });
}

void ContentController::refreshProfile() {
  if (current_user_id_.empty()) {
    profile_view_.setStatusText("Sign in first.");
    return;
  }
  profile_view_.setStatusText("Loading profile...");
  auto alive = alive_flag_;
  profile_api_.fetchProfileAsync(
      current_user_id_, [this, alive](bool ok, UserProfileModel profile,
                                      juce::String error_text) {
        if (!alive->load()) {
          return;
        }
        if (!ok) {
          profile_view_.setStatusText("Profile load failed: " + error_text);
          return;
        }
        if (profile.display_name.isNotEmpty()) {
          current_display_name_ = profile.display_name.toStdString();
        }
        current_bio_ = profile.bio.toStdString();
        profile_view_.setProfile(current_display_name_, current_bio_);
        settings_view_.setProfileFields(current_display_name_, current_bio_);
        profile_view_.setStatusText("Profile loaded");
      });
}

void ContentController::refreshManual() {
  manual_view_.setStatusText("Loading manual document...");
  auto alive = alive_flag_;
  document_service_.loadApiOrDocumentAsync(
      control_plane_base_url_, tempolink::juceapp::constants::kManualPath,
      tempolink::juceapp::constants::kManualFallbackDocPath,
      [this, alive](bool ok, juce::String text, juce::String error_text) {
        if (!alive->load()) {
          return;
        }
        if (!ok) {
          manual_view_.setStatusText("Manual load failed: " + error_text);
          manual_view_.setDocumentText("");
          return;
        }
        manual_view_.setDocumentText(text);
        manual_view_.setStatusText("Manual loaded");
      });
}

void ContentController::refreshQna() {
  qna_view_.setStatusText("Loading Q&A document...");
  auto alive = alive_flag_;
  document_service_.loadApiOrDocumentAsync(
      control_plane_base_url_, tempolink::juceapp::constants::kFaqPath,
      tempolink::juceapp::constants::kFaqFallbackDocPath,
      [this, alive](bool ok, juce::String text, juce::String error_text) {
        if (!alive->load()) {
          return;
        }
        if (!ok) {
          qna_view_.setStatusText("Q&A load failed: " + error_text);
          qna_view_.setDocumentText("");
          return;
        }
        qna_view_.setDocumentText(text);
        qna_view_.setStatusText("Q&A loaded");
      });
}

void ContentController::saveProfileSettings(const std::string& display_name,
                                            const std::string& bio) {
  if (current_user_id_.empty()) {
    settings_view_.setStatusText("Sign in first.");
    return;
  }
  settings_view_.setStatusText("Saving profile...");
  auto alive = alive_flag_;
  profile_api_.updateProfileAsync(
      current_user_id_, display_name, bio,
      [this, alive](bool ok, UserProfileModel profile, juce::String error_text) {
        if (!alive->load()) {
          return;
        }
        if (!ok) {
          settings_view_.setStatusText("Profile save failed: " + error_text);
          return;
        }
        current_display_name_ = profile.display_name.toStdString();
        current_bio_ = profile.bio.toStdString();
        profile_view_.setProfile(current_display_name_, current_bio_);
        settings_view_.setProfileFields(current_display_name_, current_bio_);
        settings_view_.setStatusText("Profile saved");
      });
}

void ContentController::applySettingsAudioFormat(int sample_rate_hz,
                                                 int buffer_samples) {
  const bool ok = session_.ConfigureAudioFormat(
      static_cast<std::uint32_t>(sample_rate_hz),
      static_cast<std::uint16_t>(buffer_samples));
  if (ok) {
    settings_view_.setStatusText("Audio format applied: " +
                                 juce::String(sample_rate_hz) + " / " +
                                 juce::String(buffer_samples));
  } else {
    settings_view_.setStatusText("Failed to apply audio format");
  }
}

void ContentController::runSettingsInputTest() {
  if (g_input_test_running.exchange(true)) {
    settings_view_.setStatusText("Input test is already running.");
    return;
  }

  settings_view_.setStatusText("Input test started. Make some sound for 2 seconds...");
  auto alive = alive_flag_;
  juce::Component::SafePointer<SettingsView> settings_safe(&settings_view_);
  const auto selected_input = session_.SelectedInputDevice();
  const auto sample_rate_hz = session_.SampleRateHz();
  const auto frame_samples = session_.FrameSamples();

  std::thread([alive, settings_safe, selected_input, sample_rate_hz, frame_samples]() {
    auto input = tempolink::audio::CreateDefaultAudioInputDevice();
    if (!input) {
      g_input_test_running.store(false);
      juce::MessageManager::callAsync([alive, settings_safe] {
        if (!alive->load()) {
          return;
        }
        if (auto* settings = settings_safe.getComponent(); settings != nullptr) {
          settings->setStatusText("Input test failed: input backend unavailable.");
        }
      });
      return;
    }

    if (!selected_input.empty()) {
      input->SelectDevice(selected_input);
    }

    std::atomic<float> peak{0.0F};
    tempolink::audio::AudioCaptureConfig capture_config{};
    capture_config.sample_rate_hz = sample_rate_hz;
    capture_config.channels = 1;
    capture_config.frame_samples = static_cast<std::uint16_t>(
        std::max<int>(16, static_cast<int>(frame_samples)));

    const bool started =
        input->Start(capture_config, [&peak](std::span<const std::int16_t> pcm) {
          std::int32_t max_abs = 0;
          for (const auto sample : pcm) {
            const auto abs_sample = std::abs(static_cast<std::int32_t>(sample));
            if (abs_sample > max_abs) {
              max_abs = abs_sample;
            }
          }
          const float normalized =
              std::clamp(static_cast<float>(max_abs) / 32768.0F, 0.0F, 1.0F);
          float current = peak.load(std::memory_order_relaxed);
          while (normalized > current &&
                 !peak.compare_exchange_weak(current, normalized,
                                             std::memory_order_relaxed)) {
          }
        });

    if (!started) {
      g_input_test_running.store(false);
      juce::MessageManager::callAsync([alive, settings_safe] {
        if (!alive->load()) {
          return;
        }
        if (auto* settings = settings_safe.getComponent(); settings != nullptr) {
          settings->setStatusText(
              "Input test failed: cannot start selected input device.");
        }
      });
      return;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    input->Stop();
    g_input_test_running.store(false);

    const int peak_percent = static_cast<int>(std::round(peak.load() * 100.0F));
    juce::MessageManager::callAsync([alive, settings_safe, peak_percent] {
      if (!alive->load()) {
        return;
      }
      if (auto* settings = settings_safe.getComponent(); settings != nullptr) {
        if (peak_percent <= 0) {
          settings->setStatusText(
              "Input test done: signal not detected (check gain/device).");
        } else {
          settings->setStatusText("Input test done: peak " +
                                  juce::String(peak_percent) + "%");
        }
      }
    });
  }).detach();
}

void ContentController::runSettingsOutputTest() {
  if (g_output_test_running.exchange(true)) {
    settings_view_.setStatusText("Output test is already running.");
    return;
  }

  settings_view_.setStatusText("Output test started: playing 440Hz tone...");
  auto alive = alive_flag_;
  juce::Component::SafePointer<SettingsView> settings_safe(&settings_view_);
  const auto selected_output = session_.SelectedOutputDevice();
  const auto sample_rate_hz = session_.SampleRateHz();
  const auto frame_samples = session_.FrameSamples();

  std::thread([alive, settings_safe, selected_output, sample_rate_hz,
               frame_samples]() {
    auto output = tempolink::audio::CreateDefaultAudioOutputDevice();
    if (!output) {
      g_output_test_running.store(false);
      juce::MessageManager::callAsync([alive, settings_safe] {
        if (!alive->load()) {
          return;
        }
        if (auto* settings = settings_safe.getComponent(); settings != nullptr) {
          settings->setStatusText("Output test failed: output backend unavailable.");
        }
      });
      return;
    }

    if (!selected_output.empty()) {
      output->SelectDevice(selected_output);
    }

    tempolink::audio::AudioPlaybackConfig playback_config{};
    playback_config.sample_rate_hz = sample_rate_hz;
    playback_config.channels = 1;
    playback_config.frame_samples = static_cast<std::uint16_t>(
        std::max<int>(16, static_cast<int>(frame_samples)));

    if (!output->Start(playback_config)) {
      g_output_test_running.store(false);
      juce::MessageManager::callAsync([alive, settings_safe] {
        if (!alive->load()) {
          return;
        }
        if (auto* settings = settings_safe.getComponent(); settings != nullptr) {
          settings->setStatusText(
              "Output test failed: cannot start selected output device.");
        }
      });
      return;
    }

    constexpr float kToneHz = 440.0F;
    constexpr float kToneAmp = 0.22F;
    constexpr int kToneFrames = 35;
    const float phase_step =
        2.0F * juce::MathConstants<float>::pi * kToneHz /
        static_cast<float>(playback_config.sample_rate_hz);
    float phase = 0.0F;
    std::vector<std::int16_t> frame(playback_config.frame_samples, 0);
    const auto sleep_time = std::chrono::microseconds(
        static_cast<std::int64_t>(1000000LL * playback_config.frame_samples /
                                  std::max<std::uint32_t>(1, playback_config.sample_rate_hz)));

    for (int n = 0; n < kToneFrames; ++n) {
      for (auto& sample : frame) {
        sample = static_cast<std::int16_t>(
            std::round(std::sin(phase) * kToneAmp * 32767.0F));
        phase += phase_step;
        if (phase > 2.0F * juce::MathConstants<float>::pi) {
          phase -= 2.0F * juce::MathConstants<float>::pi;
        }
      }
      output->PlayFrame(std::span<const std::int16_t>(frame.data(), frame.size()));
      std::this_thread::sleep_for(sleep_time);
    }

    output->Stop();
    g_output_test_running.store(false);
    juce::MessageManager::callAsync([alive, settings_safe] {
      if (!alive->load()) {
        return;
      }
      if (auto* settings = settings_safe.getComponent(); settings != nullptr) {
        settings->setStatusText("Output test done.");
      }
    });
  }).detach();
}

void ContentController::refreshSettingsView() {
  const auto input_devices = session_.AvailableInputDevices();
  const auto output_devices = session_.AvailableOutputDevices();

  settings_view_.setDevices(input_devices, session_.SelectedInputDevice(),
                            output_devices, session_.SelectedOutputDevice());
  settings_view_.setAudioFormat(static_cast<int>(session_.SampleRateHz()),
                                static_cast<int>(session_.FrameSamples()));
  settings_view_.setProfileFields(current_display_name_, current_bio_);
  if (input_devices.empty() || output_devices.empty()) {
    settings_view_.setStatusText(
        "오디오 장치를 찾을 수 없습니다. 장치 연결 후 새로고침하세요.");
    return;
  }
  settings_view_.setStatusText("Configure device/sample rate/buffer and apply.");
}
