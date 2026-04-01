#include <exception>

#include "tempolink/juce/MainComponent.h"

#include "tempolink/juce/app/ClientAppPresenter.h"
#include "tempolink/juce/bridge/UdpAudioBridgePort.h"
#include "tempolink/juce/config/ClientEnvConfig.h"
#include "tempolink/juce/logging/AppLogger.h"
#include "tempolink/juce/style/UiStyle.h"

MainComponent::MainComponent() {
  tempolink::juceapp::style::ThemeManager::getInstance().addListener(this);
  addAndMakeVisible(lobby_view_);
  addAndMakeVisible(login_view_);
  addAndMakeVisible(my_rooms_view_);
  addAndMakeVisible(room_entry_view_);
  addAndMakeVisible(session_view_);
  addAndMakeVisible(profile_view_);
  addAndMakeVisible(users_view_);
  addAndMakeVisible(news_view_);
  addAndMakeVisible(manual_view_);
  addAndMakeVisible(qna_view_);
  addAndMakeVisible(settings_view_);

  // Load and set Logo
  auto exe_file = juce::File::getSpecialLocation(juce::File::currentExecutableFile);
  auto project_root = exe_file.getParentDirectory().getParentDirectory().getParentDirectory(); // adjust if needed
  auto logo_file = project_root.getChildFile("Assets").getChildFile("Logos").getChildFile("icons.png");
  
  if (logo_file.existsAsFile()) {
    auto logo_img = juce::ImageFileFormat::loadFrom(logo_file);
    if (logo_img.isValid()) {
      lobby_view_.setLogoImage(logo_img);
    }
  }

  try {
    const auto env = tempolink::juceapp::config::ClientEnvConfig::Load();
    audio_bridge_ = std::make_shared<tempolink::juceapp::bridge::UdpAudioBridgePort>(
        env.bridge_host, env.bridge_client_listen_port,
        env.bridge_plugin_listen_port);
    presenter_ = std::make_unique<ClientAppPresenter>(
        lobby_view_, login_view_, my_rooms_view_, room_entry_view_, session_view_,
        profile_view_, users_view_, news_view_, manual_view_, qna_view_,
        settings_view_);
    presenter_->setAudioBridge(audio_bridge_);
    presenter_->initialize();
  } catch (const std::exception& ex) {
    tempolink::juceapp::logging::Error(
        "MainComponent initialization failed: " + juce::String(ex.what()));
    presenter_.reset();
  } catch (...) {
    tempolink::juceapp::logging::Error(
        "MainComponent initialization failed: unknown exception");
    presenter_.reset();
  }

  startTimerHz(tempolink::juceapp::style::kUiTimerHz);
}

MainComponent::~MainComponent() {
  tempolink::juceapp::style::ThemeManager::getInstance().removeListener(this);
  stopTimer();
  if (presenter_ != nullptr) {
    try {
      presenter_->shutdown();
    } catch (const std::exception& ex) {
      tempolink::juceapp::logging::Error(
          "Presenter shutdown failed: " + juce::String(ex.what()));
    } catch (...) {
      tempolink::juceapp::logging::Error(
          "Presenter shutdown failed: unknown exception");
    }
    presenter_.reset();
  }
  audio_bridge_.reset();
}

void MainComponent::themeChanged() {
  repaint();
  // Ensure all views are also repainted (some might have custom paint that doesn't rely on findColour)
  lobby_view_.repaint();
  login_view_.repaint();
  session_view_.repaint();
  settings_view_.repaint();
}

void MainComponent::resized() {
  const auto bounds = getLocalBounds();
  lobby_view_.setBounds(bounds);
  login_view_.setBounds(bounds);
  my_rooms_view_.setBounds(bounds);
  room_entry_view_.setBounds(bounds);
  session_view_.setBounds(bounds);
  profile_view_.setBounds(bounds);
  users_view_.setBounds(bounds);
  news_view_.setBounds(bounds);
  manual_view_.setBounds(bounds);
  qna_view_.setBounds(bounds);
  settings_view_.setBounds(bounds);
}

void MainComponent::timerCallback() {
  if (presenter_ != nullptr) {
    try {
      presenter_->tick();
    } catch (const std::exception& ex) {
      tempolink::juceapp::logging::Error(
          "Presenter tick failed: " + juce::String(ex.what()));
    } catch (...) {
      tempolink::juceapp::logging::Error("Presenter tick failed: unknown exception");
    }
  }
}
