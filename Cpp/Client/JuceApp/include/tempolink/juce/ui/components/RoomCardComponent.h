#pragma once

#include <functional>
#include <string>

#include <juce_gui_basics/juce_gui_basics.h>
#include "tempolink/juce/style/ThemeableComponent.h"

#include "tempolink/juce/ui/models/UiModels.h"

class RoomCardComponent final : public tempolink::juceapp::style::ThemeableComponent {
 public:
  enum class CardMode {
    Lobby,
    MyRooms
  };

  RoomCardComponent();
  void updateTheme() override;

  void setRoom(const RoomSummary& room);
  void setMode(CardMode mode);
  void setPreviewHandler(std::function<void(std::string)> on_preview);
  void setEnterHandler(std::function<void(std::string)> on_enter);
  void setEditHandler(std::function<void(std::string)> on_edit);
  void setDeleteHandler(std::function<void(std::string)> on_delete);
  void setShareHandler(std::function<void(std::string)> on_share);

  void resized() override;
  void paint(juce::Graphics& g) override;

 private:
  void refreshActionVisibility();

  CardMode mode_ = CardMode::Lobby;
  RoomSummary room_;
  std::function<void(std::string)> on_preview_;
  std::function<void(std::string)> on_enter_;
  std::function<void(std::string)> on_edit_;
  std::function<void(std::string)> on_delete_;
  std::function<void(std::string)> on_share_;

  juce::Label title_label_;
  juce::Label description_label_;
  juce::Label meta_label_;
  juce::Label host_label_;
  juce::Label latency_label_;
  juce::TextButton preview_button_{"Preview"};
  juce::TextButton enter_button_{"Enter"};
  juce::TextButton edit_button_{"Edit"};
  juce::TextButton delete_button_{"Delete"};
  juce::TextButton share_button_{"Share"};

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RoomCardComponent)
};
