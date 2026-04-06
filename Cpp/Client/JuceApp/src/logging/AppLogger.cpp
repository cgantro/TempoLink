#include "tempolink/juce/logging/AppLogger.h"

#include <algorithm>
#include <cstdint>
#include <mutex>

namespace tempolink::juceapp::logging {

namespace {

constexpr int kMaxLogFiles = 14;
std::mutex g_logger_mutex;
std::unique_ptr<juce::FileLogger> g_file_logger;
juce::String g_current_log_path;

void CleanupOldLogs(const juce::File& dir);

std::unique_ptr<juce::FileLogger> TryCreateLoggerIn(
    const juce::File& dir,
    const juce::String& app_name,
    const juce::String& app_version,
    juce::String& out_path) {
  if (!dir.createDirectory()) {
    return nullptr;
  }
  CleanupOldLogs(dir);

  auto logger = std::unique_ptr<juce::FileLogger>(juce::FileLogger::createDateStampedLogger(
      dir.getFullPathName(), "client", ".log",
      app_name + " v" + app_version + " session started"));
  if (logger == nullptr) {
    return nullptr;
  }

  out_path = logger->getLogFile().getFullPathName();
  return logger;
}

void CleanupOldLogs(const juce::File& dir) {
  juce::Array<juce::File> files;
  dir.findChildFiles(files, juce::File::findFiles, false, "*.log");
  if (files.size() <= kMaxLogFiles) {
    return;
  }

  std::vector<juce::File> sorted;
  sorted.reserve(static_cast<std::size_t>(files.size()));
  for (const auto& file : files) {
    sorted.push_back(file);
  }
  std::sort(sorted.begin(), sorted.end(),
            [](const juce::File& a, const juce::File& b) {
              return a.getLastModificationTime() < b.getLastModificationTime();
            });

  const int remove_count = static_cast<int>(sorted.size()) - kMaxLogFiles;
  for (int i = 0; i < remove_count; ++i) {
    sorted[static_cast<std::size_t>(i)].deleteFile();
  }
}

void WriteUnlocked(const juce::String& level, const juce::String& message) {
  const auto tid_value = static_cast<std::uint64_t>(
      reinterpret_cast<std::uintptr_t>(juce::Thread::getCurrentThreadId()));
  const juce::String tid_hex = juce::String::toHexString(
      static_cast<juce::int64>(tid_value));
  const juce::String line =
      "[" + level + "][tid=0x" + tid_hex + "] " + message;
  if (g_file_logger != nullptr) {
    g_file_logger->logMessage(line);
    return;
  }

  // Fallback path when logger initialization failed or already shut down.
  juce::Logger::outputDebugString(line);
}

void Write(const juce::String& level, const juce::String& message) {
  std::lock_guard<std::mutex> lock(g_logger_mutex);
  WriteUnlocked(level, message);
}

}  // namespace

void Initialize(const juce::String& app_name, const juce::String& app_version) {
  std::lock_guard<std::mutex> lock(g_logger_mutex);
  if (g_file_logger != nullptr) {
    return;
  }

  juce::Array<juce::File> candidates;
  candidates.add(LogDirectory());
  candidates.add(juce::File::getSpecialLocation(juce::File::currentExecutableFile)
                     .getParentDirectory()
                     .getChildFile("Logs"));
  candidates.add(juce::File::getSpecialLocation(juce::File::tempDirectory)
                     .getChildFile("TempoLink")
                     .getChildFile("Logs"));

  for (const auto& dir : candidates) {
    g_file_logger = TryCreateLoggerIn(dir, app_name, app_version, g_current_log_path);
    if (g_file_logger != nullptr) {
      g_file_logger->logMessage("[INFO] Log file path: " + g_current_log_path);
      return;
    }
  }

  g_current_log_path = {};
}

void Shutdown() {
  std::lock_guard<std::mutex> lock(g_logger_mutex);
  g_file_logger.reset();
  g_current_log_path = {};
}

void Info(const juce::String& message) { Write("INFO", message); }

void Warn(const juce::String& message) { Write("WARN", message); }

void Error(const juce::String& message) { Write("ERROR", message); }

juce::File LogDirectory() {
  return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
      .getChildFile("TempoLink")
      .getChildFile("Logs");
}

juce::String CurrentLogFilePath() {
  std::lock_guard<std::mutex> lock(g_logger_mutex);
  return g_current_log_path;
}

}  // namespace tempolink::juceapp::logging
