#include <juce_gui_extra/juce_gui_extra.h>

#include "tempolink/juce/MainComponent.h"

class TempoLinkJuceApplication final : public juce::JUCEApplication {
 public:
  const juce::String getApplicationName() override {
    return "TempoLink JUCE Client";
  }

  const juce::String getApplicationVersion() override { return "0.1.0"; }

  bool moreThanOneInstanceAllowed() override { return true; }

  void initialise(const juce::String&) override {
    main_window_ = std::make_unique<MainWindow>(getApplicationName());
  }

  void shutdown() override { main_window_.reset(); }

  void systemRequestedQuit() override { quit(); }

 private:
  class MainWindow final : public juce::DocumentWindow {
   public:
    explicit MainWindow(const juce::String& name)
        : juce::DocumentWindow(name, juce::Colours::lightgrey,
                               juce::DocumentWindow::allButtons) {
      setUsingNativeTitleBar(true);
      setContentOwned(new MainComponent(), true);
      centreWithSize(1320, 780);
      setResizable(true, true);
      setVisible(true);
    }

    void closeButtonPressed() override {
      juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }
  };

  std::unique_ptr<MainWindow> main_window_;
};

START_JUCE_APPLICATION(TempoLinkJuceApplication)
