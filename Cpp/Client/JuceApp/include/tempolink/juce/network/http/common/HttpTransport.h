#pragma once

#include <string>

#include <juce_core/juce_core.h>

namespace tempolink::juceapp::network::http {

struct Request {
  std::string method = "GET";
  std::string path = "/";
  juce::String body;
  int timeout_ms = 3500;
};

struct Response {
  bool transport_ok = false;
  int status_code = 0;
  juce::String body;
  juce::String error_text;

  bool isHttpSuccess() const { return status_code >= 200 && status_code < 300; }
};

std::string NormalizeBaseUrl(std::string base_url);
std::string EscapePathSegment(const std::string& value);
std::string EncodeQueryValue(const std::string& value);
Response Perform(const std::string& base_url, const Request& request);

}  // namespace tempolink::juceapp::network::http

