#include "tempolink/net/UdpSocket.h"

#include <WinSock2.h>
#include <WS2tcpip.h>

#include <cstring>
#include <memory>
#include <mutex>
#include <span>
#include <utility>

namespace tempolink::net {

// -----------------------------------------------------------------------
// Pimpl — Windows (WinSock2) implementation
// -----------------------------------------------------------------------

struct UdpSocket::Impl {
  SOCKET socket = INVALID_SOCKET;

  struct WsaRuntime {
    bool initialized = false;

    WsaRuntime() {
      WSADATA data{};
      initialized = (WSAStartup(MAKEWORD(2, 2), &data) == 0);
    }

    ~WsaRuntime() {
      if (initialized) {
        WSACleanup();
      }
    }
  };

  static bool Startup() {
    static std::once_flag once;
    static bool initialized = false;
    static std::unique_ptr<WsaRuntime> runtime;
    std::call_once(once, [] {
      runtime = std::make_unique<WsaRuntime>();
      initialized = runtime->initialized;
    });
    return initialized;
  }

  bool IsOpen() const { return socket != INVALID_SOCKET; }

  void Close() {
    if (IsOpen()) {
      closesocket(socket);
      socket = INVALID_SOCKET;
    }
  }

  bool Open() {
    if (IsOpen()) return true;
    if (!Startup()) return false;
    socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    return socket != INVALID_SOCKET;
  }

  bool Bind(std::uint16_t port, const std::string& bind_address) {
    if (!IsOpen() && !Open()) return false;
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (bind_address == "0.0.0.0") {
      addr.sin_addr.s_addr = INADDR_ANY;
    } else {
      if (inet_pton(AF_INET, bind_address.c_str(), &addr.sin_addr) != 1)
        return false;
    }
    const int reuse = 1;
    setsockopt(socket, SOL_SOCKET, SO_REUSEADDR,
               reinterpret_cast<const char*>(&reuse), sizeof(reuse));
    return ::bind(socket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == 0;
  }

  bool SetNonBlocking(bool enabled) {
    if (!IsOpen()) return false;
    u_long mode = enabled ? 1UL : 0UL;
    return ioctlsocket(socket, FIONBIO, &mode) == 0;
  }

  bool SendTo(std::span<const std::byte> data, const std::string& host,
              std::uint16_t port) {
    if (!IsOpen()) return false;
    addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    addrinfo* result = nullptr;
    const std::string port_text = std::to_string(port);
    if (getaddrinfo(host.c_str(), port_text.c_str(), &hints, &result) != 0 ||
        result == nullptr)
      return false;
    const int sent =
        sendto(socket, reinterpret_cast<const char*>(data.data()),
               static_cast<int>(data.size()), 0, result->ai_addr,
               static_cast<int>(result->ai_addrlen));
    freeaddrinfo(result);
    return sent >= 0;
  }

  ReceiveResult ReceiveFrom(std::span<std::byte> buffer) {
    if (!IsOpen()) return {SocketStatus::Closed, 0, {}};
    if (buffer.empty()) return {SocketStatus::Error, 0, {}};
    sockaddr_in source{};
    int source_size = sizeof(source);
    const int read = recvfrom(
        socket, reinterpret_cast<char*>(buffer.data()),
        static_cast<int>(buffer.size()), 0,
        reinterpret_cast<sockaddr*>(&source), &source_size);
    if (read < 0) {
      const int error = WSAGetLastError();
      if (error == WSAEWOULDBLOCK) {
        return {SocketStatus::WouldBlock, 0, {}};
      }
      if (error == WSAENOTSOCK || error == WSAESHUTDOWN ||
          error == WSAECONNABORTED) {
        return {SocketStatus::Closed, 0, {}};
      }
      return {SocketStatus::Error, 0, {}};
    }

    ReceiveResult result;
    result.status = SocketStatus::Success;
    result.bytes_read = static_cast<std::size_t>(read);
    result.sender.address_ipv4 = ntohl(source.sin_addr.s_addr);
    result.sender.port = ntohs(source.sin_port);
    return result;
  }

  static std::string LastErrorMessage() {
    return "WSA error: " + std::to_string(WSAGetLastError());
  }
};

// -----------------------------------------------------------------------
// UdpSocket delegating interface → Impl
// -----------------------------------------------------------------------

UdpSocket::UdpSocket() : impl_(std::make_unique<Impl>()) {}

UdpSocket::~UdpSocket() {
  if (impl_) impl_->Close();
}

UdpSocket::UdpSocket(UdpSocket&& other) noexcept = default;
UdpSocket& UdpSocket::operator=(UdpSocket&& other) noexcept {
  if (this != &other) {
    if (impl_) impl_->Close();
    impl_ = std::move(other.impl_);
  }
  return *this;
}

bool UdpSocket::Open() { return impl_ && impl_->Open(); }
bool UdpSocket::Bind(std::uint16_t port, const std::string& addr) { return impl_ && impl_->Bind(port, addr); }
bool UdpSocket::SetNonBlocking(bool enabled) { return impl_ && impl_->SetNonBlocking(enabled); }
bool UdpSocket::SendTo(std::span<const std::byte> data, const std::string& host, std::uint16_t port) { return impl_ && impl_->SendTo(data, host, port); }
ReceiveResult UdpSocket::ReceiveFrom(std::span<std::byte> buffer) {
  if (!impl_) {
    return {SocketStatus::Closed, 0, {}};
  }
  return impl_->ReceiveFrom(buffer);
}
bool UdpSocket::IsOpen() const { return impl_ && impl_->IsOpen(); }
void UdpSocket::Close() { if (impl_) impl_->Close(); }
std::string UdpSocket::LastErrorMessage() { return Impl::LastErrorMessage(); }

}  // namespace tempolink::net
