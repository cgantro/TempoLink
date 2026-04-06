#pragma once

#include <string>

#include <juce_core/juce_core.h>

namespace tempolink::juceapp::network {

class ISocketStreamChannel {
 public:
  virtual ~ISocketStreamChannel() = default;

  virtual bool Initialize(const std::string& host, juce::String* error_text) = 0;
  virtual void Shutdown() = 0;
  virtual int ReadSome(void* out, int bytes, int timeout_ms) = 0;
  virtual int WriteSome(const void* in, int bytes, int timeout_ms) = 0;
};

}  // namespace tempolink::juceapp::network

