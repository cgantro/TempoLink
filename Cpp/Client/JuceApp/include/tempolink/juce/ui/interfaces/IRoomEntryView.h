#pragma once

#include <string>
#include <vector>
#include <functional>

#include "tempolink/juce/ui/models/UiModels.h"

class IRoomEntryView {
 public:
  struct EntrySelection {
    std::string room_code;
    std::string part_label;
    std::string input_device;
    std::string output_device;
  };

  virtual ~IRoomEntryView() = default;

  virtual void setRoom(const RoomSummary& room) = 0;
  virtual void setStatusText(const std::string& status_text) = 0;
  virtual void setInputDevices(const std::vector<std::string>& devices,
                               const std::string& selected_device) = 0;
  virtual void setOutputDevices(const std::vector<std::string>& devices,
                                const std::string& selected_device) = 0;
  virtual void setSelectedPart(const std::string& part_label) = 0;
  
  virtual void setOnBack(std::function<void()> on_back) = 0;
  virtual void setOnJoin(std::function<void(EntrySelection)> on_join) = 0;
  virtual void setOnPartChanged(std::function<void(std::string)> on_part_changed) = 0;
  virtual void setOnInputDeviceChanged(std::function<void(std::string)> on_input_changed) = 0;
  virtual void setOnOutputDeviceChanged(std::function<void(std::string)> on_output_changed) = 0;
  virtual void setOnOpenAudioSettings(std::function<void()> on_open_audio_settings) = 0;
};
