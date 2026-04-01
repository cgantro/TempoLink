#include "tempolink/juce/network/ice/IceConfigClient.h"

#include <thread>
#include <utility>

#include <juce_events/juce_events.h>

#include "tempolink/juce/constants/ClientText.h"
#include "tempolink/juce/network/http/common/HttpTransport.h"
#include "tempolink/juce/network/ice/IceConfigJsonMapper.h"

namespace {

namespace http = tempolink::juceapp::network::http;
namespace iceconfig = tempolink::juceapp::network::iceconfig;

}  // namespace

IceConfigClient::IceConfigClient(std::string base_url)
    : base_url_(http::NormalizeBaseUrl(std::move(base_url))) {}

void IceConfigClient::fetchIceConfigAsync(Callback callback) const {
  std::thread([base_url = base_url_, callback = std::move(callback)]() mutable {
    juce::String error_text;
    IceConfigSnapshot snapshot;

    http::Request request;
    request.method = "GET";
    request.path = "/api/network/ice";
    request.timeout_ms = 3500;
    const http::Response response = http::Perform(base_url, request);
    if (!response.transport_ok) {
      error_text = tempolink::juceapp::text::kIceApiConnectFailed;
    } else if (response.isHttpSuccess()) {
      snapshot = iceconfig::ParseIceConfig(response.body);
    } else {
      error_text = juce::String(tempolink::juceapp::text::kIceApiHttpErrorPrefix) +
                   juce::String(response.status_code);
    }

    juce::MessageManager::callAsync(
        [callback = std::move(callback), snapshot = std::move(snapshot), error_text]() mutable {
          callback(error_text.isEmpty(), std::move(snapshot), error_text);
        });
  }).detach();
}

