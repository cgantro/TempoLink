#include <exception>
#include <cstdlib>
#if defined(_MSC_VER) && defined(_DEBUG)
#include <crtdbg.h>
#endif

#include <juce_gui_extra/juce_gui_extra.h>

#include "tempolink/juce/MainComponent.h"
#include "tempolink/juce/logging/AppLogger.h"
#include "tempolink/juce/config/ClientEnvConfig.h"
#include "tempolink/juce/style/TempoLinkLookAndFeel.h"

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

    juce::String selected_env = "deploy"; // 기본값은 실제 서비스(배포) 환경
#if defined(_DEBUG) || defined(DEBUG)
    selected_env = "local"; // 개발자용 Debug 빌드일 경우 로컬 환경을 기본으로 사용
#endif

    juce::StringArray args = juce::JUCEApplication::getInstance()->getCommandLineParameterArray();
    for (const auto& arg : args) {
      if (arg == "--env=local") {
        selected_env = "local";
      } else if (arg == "--env=deploy") {
        selected_env = "deploy";
      }
    }

    tempolink::juceapp::config::ClientEnvConfig::SetEnvironment(selected_env.toStdString());
    tempolink::juceapp::logging::Info("Starting application with environment: " + selected_env);

    juce::LookAndFeel::setDefaultLookAndFeel(&look_and_feel_);
    main_window_ = std::make_unique<MainWindow>(getApplicationName());
  }

  void shutdown() override {
    main_window_.reset();
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
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
  tempolink::juceapp::style::TempoLinkLookAndFeel look_and_feel_;
};

START_JUCE_APPLICATION(TempoLinkJuceApplication)
