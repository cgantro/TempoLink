#include "tempolink/juce/network/http/common/DocumentContentService.h"

#include <thread>
#include <utility>

#include <juce_events/juce_events.h>

#include "tempolink/juce/network/http/common/HttpTransport.h"
#include "tempolink/juce/network/http/common/JsonParse.h"

namespace {

namespace http = tempolink::juceapp::network::http;
namespace jsonparse = tempolink::juceapp::jsonparse;

juce::File ResolveRepoRoot() {
  juce::File dir = juce::File::getSpecialLocation(juce::File::currentExecutableFile)
                       .getParentDirectory();
  for (int i = 0; i < 10; ++i) {
    if (dir.getChildFile(".git").exists() ||
        dir.getChildFile("Docs").isDirectory()) {
      return dir;
    }
    auto next = dir.getParentDirectory();
    if (next == dir) {
      break;
    }
    dir = next;
  }
  return {};
}

bool LoadDocumentFromRelativePath(const std::string& relative_path,
                                  juce::String& text,
                                  juce::String& error_text) {
  const juce::File root = ResolveRepoRoot();
  if (!root.isDirectory()) {
    error_text = "Repository root not found.";
    return false;
  }

  const juce::File doc = root.getChildFile(relative_path);
  if (!doc.existsAsFile()) {
    error_text = "Document not found: " + juce::String(relative_path);
    return false;
  }

  text = doc.loadFileAsString();
  if (text.isEmpty()) {
    error_text = "Document is empty: " + juce::String(relative_path);
    return false;
  }
  return true;
}

bool TryParseApiDocument(const juce::String& body, juce::String& text) {
  const auto json = jsonparse::Parse(body);
  if (json.is_object()) {
    const auto content = jsonparse::String(json, "content", "");
    if (!content.empty()) {
      text = juce::String(content);
      return true;
    }
    const auto markdown = jsonparse::String(json, "markdown", "");
    if (!markdown.empty()) {
      text = juce::String(markdown);
      return true;
    }
    const auto value = jsonparse::String(json, "text", "");
    if (!value.empty()) {
      text = juce::String(value);
      return true;
    }
  }
  if (json.is_string()) {
    text = juce::String(json.get<std::string>());
    return text.isNotEmpty();
  }

  text = body.trim();
  return text.isNotEmpty();
}

}  // namespace

void DocumentContentService::loadDocumentAsync(const std::string& relative_path,
                                               Callback callback) const {
  std::thread([relative_path, callback = std::move(callback)]() mutable {
    juce::String text;
    juce::String error_text;
    LoadDocumentFromRelativePath(relative_path, text, error_text);

    juce::MessageManager::callAsync(
        [callback = std::move(callback), text, error_text]() mutable {
          callback(error_text.isEmpty(), text, error_text);
        });
  }).detach();
}

void DocumentContentService::loadApiOrDocumentAsync(
    const std::string& base_url, const std::string& api_path,
    const std::string& fallback_relative_path, Callback callback) const {
  std::thread([base_url, api_path, fallback_relative_path,
               callback = std::move(callback)]() mutable {
    juce::String text;
    juce::String error_text;

    http::Request request;
    request.method = "GET";
    request.path = api_path;
    request.timeout_ms = 3500;

    const auto response = http::Perform(base_url, request);
    if (response.transport_ok && response.isHttpSuccess() &&
        TryParseApiDocument(response.body, text)) {
      juce::MessageManager::callAsync(
          [callback = std::move(callback), text]() mutable {
            callback(true, text, {});
          });
      return;
    }

    juce::String fallback_error;
    if (!LoadDocumentFromRelativePath(fallback_relative_path, text, fallback_error)) {
      if (response.transport_ok && !response.isHttpSuccess()) {
        error_text = "API HTTP " + juce::String(response.status_code) + " / " + fallback_error;
      } else if (response.transport_ok) {
        error_text = "API returned empty content / " + fallback_error;
      } else {
        error_text = response.error_text + " / " + fallback_error;
      }
    }

    juce::MessageManager::callAsync(
        [callback = std::move(callback), text, error_text]() mutable {
          callback(error_text.isEmpty(), text, error_text);
        });
  }).detach();
}
