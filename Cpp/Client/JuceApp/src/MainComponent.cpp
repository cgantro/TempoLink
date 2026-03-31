#include "tempolink/juce/MainComponent.h"

#include "tempolink/juce/app/ClientAppPresenter.h"
#include "tempolink/juce/style/UiStyle.h"

MainComponent::MainComponent() {
  addAndMakeVisible(lobby_view_);
  addAndMakeVisible(room_entry_view_);
  addAndMakeVisible(session_view_);

  presenter_ = std::make_unique<ClientAppPresenter>(lobby_view_, room_entry_view_,
                                                     session_view_);
  presenter_->initialize();
  startTimerHz(tempolink::juceapp::style::kUiTimerHz);
}

MainComponent::~MainComponent() {
  stopTimer();
  if (presenter_ != nullptr) {
    presenter_->shutdown();
    presenter_.reset();
  }
}

void MainComponent::resized() {
  const auto bounds = getLocalBounds();
  lobby_view_.setBounds(bounds);
  room_entry_view_.setBounds(bounds);
  session_view_.setBounds(bounds);
}

void MainComponent::timerCallback() {
  if (presenter_ != nullptr) {
    presenter_->tick();
  }
}
