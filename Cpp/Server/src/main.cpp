#include <csignal>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string_view>

#include "tempolink/server/UdpRelayServer.h"

namespace {

tempolink::server::UdpRelayServer* g_server = nullptr;

void OnSignal(int signal) {
  if (signal == SIGINT || signal == SIGTERM) {
    if (g_server != nullptr) {
      g_server->Stop();
    }
  }
}

std::uint16_t ParsePort(int argc, char** argv) {
  std::uint16_t port = 40000;
  for (int i = 1; i < argc; ++i) {
    const std::string_view arg = argv[i];
    if (arg == "--port" && i + 1 < argc) {
      port = static_cast<std::uint16_t>(std::strtoul(argv[i + 1], nullptr, 10));
      ++i;
    }
  }
  return port;
}

}  // namespace

int main(int argc, char** argv) {
  const auto port = ParsePort(argc, argv);
  tempolink::server::UdpRelayServer server(port);
  g_server = &server;

  std::signal(SIGINT, OnSignal);
  std::signal(SIGTERM, OnSignal);

  if (!server.Start()) {
    return 1;
  }

  server.Run();
  std::cout << "[server] stopped\n";
  return 0;
}

