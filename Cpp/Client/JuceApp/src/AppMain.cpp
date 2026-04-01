#include <exception>
#include <cstdlib>
#if defined(_MSC_VER) && defined(_DEBUG)
#include <crtdbg.h>
#endif

#include <juce_gui_extra/juce_gui_extra.h>

#include "tempolink/juce/MainComponent.h"
#include "tempolink/juce/logging/AppLogger.h"

class TempoLinkJuceApplication final : public juce::JUCEApplication {
 public:
  const juce::String getApplicationName() override {
    return "TempoLink JUCE Client";
  }

  const juce::String getApplicationVersion() override { return "0.1.0"; }

  bool moreThanOneInstanceAllowed() override { return true; }

  void initialise(const juce::String&) override {
#if defined(_MSC_VER) && defined(_DEBUG)
    const int dbg_flags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
    int new_flags = dbg_flags | _CRTDBG_ALLOC_MEM_DF;
    if (const char* env = std::getenv("TEMPOLINK_HEAPCHECK_ALWAYS");
        env != nullptr && juce::String(env).trim() == "1") {
      new_flags |= _CRTDBG_CHECK_ALWAYS_DF;
    }
    _CrtSetDbgFlag(new_flags);
#endif
    tempolink::juceapp::logging::Initialize(getApplicationName(),
                                            getApplicationVersion());
    main_window_ = std::make_unique<MainWindow>(getApplicationName());
  }

  void shutdown() override {
    main_window_.reset();
    tempolink::juceapp::logging::Shutdown();
  }

  void systemRequestedQuit() override { quit(); }

  void unhandledException(const std::exception* exception,
                          const juce::String& source_file,
                          int line_number) override {
    juce::String message = "Unhandled exception";
    if (exception != nullptr) {
      message << ": " << juce::String(exception->what());
    }
    if (source_file.isNotEmpty()) {
      message << " @ " << source_file << ":" << juce::String(line_number);
    }
    tempolink::juceapp::logging::Error(message);
  }

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
