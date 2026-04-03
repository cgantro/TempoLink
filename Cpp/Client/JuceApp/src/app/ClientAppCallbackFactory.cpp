#include "tempolink/juce/app/ClientAppCallbackFactory.h"

#include <utility>

namespace tempolink::juceapp::app {

RoomCommandController::Callbacks ClientAppCallbackFactory::CreateRoomCommandCallbacks(
    tempolink::juceapp::di::ViewRegistry& views,
    RoomCommandUiCoordinator& room_command_ui_coordinator,
    std::function<void()> refresh_room_views,
    std::function<void()> fetch_rooms_from_api,
    std::function<void(const std::string&)> set_lobby_status_text,
    std::function<void(const std::string&)> open_room_entry) {
  RoomCommandController::Callbacks callbacks;
  callbacks.refresh_room_views = std::move(refresh_room_views);
  callbacks.fetch_rooms_from_api = std::move(fetch_rooms_from_api);
  callbacks.set_lobby_status_text =
      [set_lobby_status_text = std::move(set_lobby_status_text)](const juce::String& text) {
        if (set_lobby_status_text) {
          set_lobby_status_text(text.toStdString());
        }
      };
  callbacks.open_room_entry = std::move(open_room_entry);
  callbacks.set_my_rooms_status_text = [&views](const std::string& text) {
    views.my_rooms_view.setStatusText(text);
  };
  callbacks.request_room_edit_payload =
      [&room_command_ui_coordinator](
          const RoomSummary& room,
          std::function<void(std::optional<RoomUpdatePayload>)> on_complete) {
        room_command_ui_coordinator.requestEditRoomPayload(room, std::move(on_complete));
      };
  callbacks.request_delete_confirmation =
      [&room_command_ui_coordinator](const std::string& room_code,
                                     std::function<void(bool)> on_complete) {
        room_command_ui_coordinator.requestDeleteConfirmation(room_code, std::move(on_complete));
      };
  callbacks.copy_to_clipboard = [&room_command_ui_coordinator](const juce::String& text) {
    room_command_ui_coordinator.copyToClipboard(text);
  };
  return callbacks;
}

SessionLifecycleCallbackBundle ClientAppCallbackFactory::CreateSessionLifecycleCallbacks(
    tempolink::juceapp::di::ViewRegistry& views,
    std::function<void(const std::string&)> set_lobby_status_text,
    std::function<void(tempolink::juceapp::app::ScreenMode)> switch_screen,
    std::function<void()> on_session_started,
    std::function<void()> on_session_ended) {
  SessionLifecycleCallbackBundle bundle;

  bundle.view_callbacks.set_login_status_text = [&views](const std::string& text) {
    views.login_view.setStatusText(text);
  };
  bundle.view_callbacks.set_room_entry_room = [&views](const RoomSummary& room) {
    views.room_entry_view.setRoom(room);
  };
  bundle.view_callbacks.set_room_entry_selected_part = [&views](const std::string& part) {
    views.room_entry_view.setSelectedPart(part);
  };
  bundle.view_callbacks.set_room_entry_input_devices =
      [&views](const std::vector<std::string>& devices, const std::string& selected) {
        views.room_entry_view.setInputDevices(devices, selected);
      };
  bundle.view_callbacks.set_room_entry_output_devices =
      [&views](const std::vector<std::string>& devices, const std::string& selected) {
        views.room_entry_view.setOutputDevices(devices, selected);
      };
  bundle.view_callbacks.set_room_entry_status_text = [&views](const std::string& text) {
    views.room_entry_view.setStatusText(text);
  };
  bundle.view_callbacks.set_session_room_title = [&views](const std::string& title) {
    views.session_view.setRoomTitle(title);
  };
  bundle.view_callbacks.set_session_connection_state = [&views](bool connected) {
    views.session_view.setConnectionState(connected);
  };
  bundle.view_callbacks.set_session_status_text = [&views](const std::string& text) {
    views.session_view.setStatusText(text);
  };
  bundle.view_callbacks.set_session_mute = [&views](bool muted) {
    views.session_view.setMute(muted);
  };
  bundle.view_callbacks.set_session_input_level = [&views](float level) {
    views.session_view.setInputLevel(level);
  };
  bundle.view_callbacks.set_session_input_gain = [&views](float gain) {
    views.session_view.setInputGain(gain);
  };
  bundle.view_callbacks.set_session_input_reverb = [&views](float amount) {
    views.session_view.setInputReverb(amount);
  };
  bundle.view_callbacks.set_session_recording = [&views](bool recording) {
    views.session_view.setRecording(recording);
  };
  bundle.view_callbacks.set_session_audio_file_active = [&views](bool active) {
    views.session_view.setAudioFileActive(active);
  };
  bundle.view_callbacks.set_session_metronome_enabled = [&views](bool enabled) {
    views.session_view.setMetronomeEnabled(enabled);
  };
  bundle.view_callbacks.set_session_metronome_bpm = [&views](int bpm) {
    views.session_view.setMetronomeBpm(bpm);
  };
  bundle.view_callbacks.set_session_metronome_tone = [&views](int tone) {
    views.session_view.setMetronomeTone(tone);
  };
  bundle.view_callbacks.set_session_master_volume = [&views](float volume) {
    views.session_view.setMasterVolume(volume);
  };

  bundle.routing_callbacks.set_lobby_status_text = std::move(set_lobby_status_text);
  bundle.routing_callbacks.switch_screen = std::move(switch_screen);
  bundle.routing_callbacks.on_session_started = std::move(on_session_started);
  bundle.routing_callbacks.on_session_ended = std::move(on_session_ended);

  return bundle;
}

}  // namespace tempolink::juceapp::app
