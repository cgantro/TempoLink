#include "tempolink/juce/network/http/common/HttpTransport.h"

#include <cctype>
#include <exception>
#include <string_view>

#include <httplib.h>

namespace tempolink::juceapp::network::http {

namespace {

struct Endpoint {
  std::string host;
  int port = 80;
  bool use_tls = false;
  bool valid = false;
};

bool IsUnreserved(unsigned char c) {
  return std::isalnum(c) != 0 || c == '-' || c == '_' || c == '.' || c == '~';
}

std::string PercentEncode(std::string_view input) {
  static constexpr char kHex[] = "0123456789ABCDEF";
  std::string encoded;
  encoded.reserve(input.size() * 3);
  for (unsigned char c : input) {
    if (IsUnreserved(c)) {
      encoded.push_back(static_cast<char>(c));
      continue;
    }
    encoded.push_back('%');
    encoded.push_back(kHex[(c >> 4) & 0x0F]);
    encoded.push_back(kHex[c & 0x0F]);
  }
  return encoded;
}

template <typename ClientT>
httplib::Result SendByMethod(ClientT& client, const Request& request) {
  const httplib::Headers headers{{"Accept", "application/json"}};
  const std::string path = request.path.empty() ? std::string("/") : request.path;
  const std::string body = request.body.toStdString();

  if (request.method == "GET") {
    return client.Get(path, headers);
  }
  if (request.method == "POST") {
    return client.Post(path, headers, body, "application/json");
  }
  if (request.method == "PUT") {
    return client.Put(path, headers, body, "application/json");
  }

  httplib::Request native_request;
  native_request.method = request.method;
  native_request.path = path;
  native_request.body = body;
  native_request.headers = headers;
  if (!native_request.body.empty()) {
    native_request.set_header("Content-Type", "application/json");
  }
  return client.send(native_request);
}

Endpoint ParseEndpoint(const std::string& base_url) {
  Endpoint endpoint;
  std::string value = base_url;
  constexpr std::string_view kHttpPrefix = "http://";
  constexpr std::string_view kHttpsPrefix = "https://";
  if (value.rfind(kHttpsPrefix, 0) == 0) {
    endpoint.use_tls = true;
    value = value.substr(kHttpsPrefix.size());
  } else
  if (value.rfind(kHttpPrefix, 0) == 0) {
    value = value.substr(kHttpPrefix.size());
  }

  const std::size_t slash = value.find('/');
  if (slash != std::string::npos) {
    value = value.substr(0, slash);
  }
  if (value.empty()) {
    return endpoint;
  }

  const std::size_t colon = value.rfind(':');
  if (colon == std::string::npos) {
    endpoint.host = value;
    endpoint.port = endpoint.use_tls ? 443 : 80;
    endpoint.valid = true;
    return endpoint;
  }
  if (colon == 0 || colon + 1 >= value.size()) {
    return endpoint;
  }

  endpoint.host = value.substr(0, colon);
  const std::string port_text = value.substr(colon + 1);
  int port = 0;
  try {
    port = std::stoi(port_text);
  } catch (const std::exception&) {
    return endpoint;
  }
  if (port <= 0 || port > 65535) {
    return endpoint;
  }
  endpoint.port = port;
  endpoint.valid = !endpoint.host.empty();
  return endpoint;
}

template <typename ClientT>
void ConfigureClient(ClientT& client, const Request& request) {
  client.set_connection_timeout(request.timeout_ms / 1000,
                                (request.timeout_ms % 1000) * 1000);
  client.set_read_timeout(request.timeout_ms / 1000,
                          (request.timeout_ms % 1000) * 1000);
  client.set_write_timeout(request.timeout_ms / 1000,
                           (request.timeout_ms % 1000) * 1000);
  client.set_keep_alive(false);
}

template <typename ClientT>
Response PerformWithClient(ClientT& client,
                           const Request& request,
                           Response response) {
  if (!client.is_valid()) {
    response.error_text = "Invalid control-plane URL";
    return response;
  }

  ConfigureClient(client, request);

  const auto result = SendByMethod(client, request);
  if (!result) {
    response.error_text = juce::String("HTTP transport error: ") +
                          juce::String(httplib::to_string(result.error()));
    return response;
  }

  response.transport_ok = true;
  response.status_code = result->status;
  response.body = juce::String(result->body);
  return response;
}

}  // namespace

std::string NormalizeBaseUrl(std::string base_url) {
  while (!base_url.empty() && base_url.back() == '/') {
    base_url.pop_back();
  }
  return base_url;
}

std::string EscapePathSegment(const std::string& value) {
  return PercentEncode(value);
}

std::string EncodeQueryValue(const std::string& value) {
  return PercentEncode(value);
}

Response Perform(const std::string& base_url, const Request& request) {
  Response response;
  try {
    const Endpoint endpoint = ParseEndpoint(base_url);
    if (!endpoint.valid) {
      response.error_text = "Invalid control-plane URL";
      return response;
    }

    if (endpoint.use_tls) {
#if defined(CPPHTTPLIB_OPENSSL_SUPPORT)
      httplib::SSLClient client(endpoint.host, endpoint.port);
      return PerformWithClient(client, request, std::move(response));
#else
      response.error_text = "HTTPS is not supported in this build.";
      return response;
#endif
    }

    httplib::Client client(endpoint.host, endpoint.port);
    return PerformWithClient(client, request, std::move(response));
  } catch (const std::exception& ex) {
    response.error_text =
        juce::String("HTTP client exception: ") + juce::String(ex.what());
    return response;
  }
}

}  // namespace tempolink::juceapp::network::http
