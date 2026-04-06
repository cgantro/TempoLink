#include "tempolink/juce/config/ClientEnvConfig.h"
  
#include <cstdlib>
#include <fstream>
#include <unordered_set>
#include <string>
#include <unordered_map>

#include <juce_core/juce_core.h>

#include "tempolink/bridge/UdpBridgeProtocol.h"
#include "tempolink/config/NetworkConstants.h"
#include "tempolink/juce/constants/AuthConstants.h"

namespace tempolink::juceapp::config {

static std::string g_current_env = "local";

namespace {

juce::File ResolveRepoRoot() {
  juce::File dir = juce::File::getSpecialLocation(juce::File::currentExecutableFile)
                       .getParentDirectory();
  for (int i = 0; i < 10; ++i) {
    if (dir.getChildFile(".git").exists() || dir.getChildFile("Cpp").isDirectory()) {
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

std::string TrimCopy(std::string value) {
  const auto begin = value.find_first_not_of(" \t\r\n");
  if (begin == std::string::npos) {
    return {};
  }
  const auto end = value.find_last_not_of(" \t\r\n");
  value = value.substr(begin, end - begin + 1);
  if (value.size() >= 2 &&
      ((value.front() == '"' && value.back() == '"') ||
       (value.front() == '\'' && value.back() == '\''))) {
    value = value.substr(1, value.size() - 2);
  }
  return value;
}

std::unordered_map<std::string, std::string> LoadDotEnvMap() {
  std::unordered_map<std::string, std::string> kv;

  std::string env_file_name = ".env";
  const std::string env_suffix = ClientEnvConfig::GetEnvironment();
  if (!env_suffix.empty()) {
    env_file_name += "." + env_suffix;
  }

  juce::File env_file;
  const auto cwd_file =
      juce::File::getCurrentWorkingDirectory().getChildFile(env_file_name);
  if (cwd_file.existsAsFile()) {
    env_file = cwd_file;
  }

  // Installed app execution can start from arbitrary working directories.
  // Prefer the executable directory as a stable deploy-time location.
  if (!env_file.existsAsFile()) {
    const auto exe_file =
        juce::File::getSpecialLocation(juce::File::currentExecutableFile)
            .getParentDirectory()
            .getChildFile(env_file_name);
    if (exe_file.existsAsFile()) {
      env_file = exe_file;
    }
  }

  if (!env_file.existsAsFile()) {
    const auto root = ResolveRepoRoot();
    if (root.isDirectory()) {
      env_file = root.getChildFile(env_file_name);
    }
  }
  if (!env_file.existsAsFile()) {
    return kv;
  }

  std::ifstream in(env_file.getFullPathName().toStdString());
  if (!in.is_open()) {
    return kv;
  }

  std::string line;
  while (std::getline(in, line)) {
    if (!line.empty() && line.back() == '\r') {
      line.pop_back();
    }
    const auto trimmed = TrimCopy(line);
    if (trimmed.empty() || trimmed[0] == '#') {
      continue;
    }
    const auto eq = trimmed.find('=');
    if (eq == std::string::npos || eq == 0) {
      continue;
    }
    auto key = TrimCopy(trimmed.substr(0, eq));
    auto value = TrimCopy(trimmed.substr(eq + 1));
    if (!key.empty()) {
      kv[key] = value;
    }
  }

  return kv;
}

std::string ReadString(const std::unordered_map<std::string, std::string>& dotenv,
                       const char* key,
                       const std::string& fallback) {
  if (const char* env = std::getenv(key); env != nullptr && env[0] != '\0') {
    return env;
  }
  const auto it = dotenv.find(key);
  if (it != dotenv.end() && !it->second.empty()) {
    return it->second;
  }
  return fallback;
}

std::uint16_t ReadPort(const std::unordered_map<std::string, std::string>& dotenv,
                       const char* key,
                       std::uint16_t fallback) {
  const auto value = ReadString(dotenv, key, "");
  if (value.empty()) {
    return fallback;
  }
  try {
    const int parsed = std::stoi(value);
    if (parsed > 0 && parsed <= 65535) {
      return static_cast<std::uint16_t>(parsed);
    }
  } catch (...) {
  }
  return fallback;
}

bool IsBrowserBlockedPort(std::uint16_t port) {
  static const std::unordered_set<std::uint16_t> kBlockedPorts = {
      1,   7,    9,    11,   13,   15,   17,   19,   20,   21,   22,   23,
      25,  37,   42,   43,   53,   69,   77,   79,   87,   95,   101,  102,
      103, 104,  109,  110,  111,  113,  115,  117,  119,  123,  135,  137,
      139, 143,  161,  179,  389,  427,  465,  512,  513,  514,  515,  526,
      530, 531,  532,  540,  548,  554,  556,  563,  587,  601,  636,  989,
      990, 993,  995,  1719, 1720, 1723, 2049, 3659, 4045, 5060, 5061, 6000,
      6566, 6665, 6666, 6667, 6668, 6669, 6697, 10080};
  return kBlockedPorts.find(port) != kBlockedPorts.end();
}

std::uint16_t SanitizeCallbackPort(std::uint16_t port, std::uint16_t fallback) {
  if (port < 1024 || IsBrowserBlockedPort(port)) {
    return fallback;
  }
  return port;
}

}  // namespace

void ClientEnvConfig::SetEnvironment(const std::string& env) {
  g_current_env = env;
}

std::string ClientEnvConfig::GetEnvironment() {
  return g_current_env;
}

ClientEnvConfig ClientEnvConfig::Load() {
  const auto dotenv = LoadDotEnvMap();
  ClientEnvConfig config;

  config.control_plane_base_url =
      ReadString(dotenv, "TEMPOLINK_CONTROL_PLANE_BASE_URL",
                 tempolink::config::kDefaultControlPlaneBaseUrl);
  config.control_plane_host =
      ReadString(dotenv, "TEMPOLINK_CONTROL_PLANE_HOST",
                 tempolink::config::kDefaultControlPlaneHost);
  config.control_plane_port =
      ReadPort(dotenv, "TEMPOLINK_CONTROL_PLANE_PORT",
               tempolink::config::kDefaultControlPlanePort);

  config.default_relay_host =
      ReadString(dotenv, "TEMPOLINK_DEFAULT_RELAY_HOST",
                 tempolink::config::kDefaultRelayHost);
  config.default_relay_port =
      ReadPort(dotenv, "TEMPOLINK_DEFAULT_RELAY_PORT",
               tempolink::config::kDefaultRelayPort);

  config.oauth_callback_host =
      ReadString(dotenv, "TEMPOLINK_OAUTH_CALLBACK_HOST",
                 tempolink::juceapp::constants::kOAuthCallbackHost);
  config.oauth_callback_port =
      SanitizeCallbackPort(
          ReadPort(dotenv, "TEMPOLINK_OAUTH_CALLBACK_PORT",
                   tempolink::juceapp::constants::kOAuthCallbackPort),
          tempolink::juceapp::constants::kOAuthCallbackPort);
  config.oauth_callback_path =
      ReadString(dotenv, "TEMPOLINK_OAUTH_CALLBACK_PATH",
                 tempolink::juceapp::constants::kOAuthCallbackPath);

  config.bridge_host =
      ReadString(dotenv, "TEMPOLINK_BRIDGE_HOST",
                 tempolink::bridge::kDefaultBridgeHost);
  config.bridge_client_listen_port =
      ReadPort(dotenv, "TEMPOLINK_BRIDGE_CLIENT_LISTEN_PORT",
               tempolink::bridge::kDefaultClientListenPort);
  config.bridge_plugin_listen_port =
      ReadPort(dotenv, "TEMPOLINK_BRIDGE_PLUGIN_LISTEN_PORT",
               tempolink::bridge::kDefaultPluginListenPort);

  return config;
}

}  // namespace tempolink::juceapp::config
