#include "tempolink/net/UdpSocket.h"

#ifdef _WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#else
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include <cstring>
#include <cerrno>
#include <mutex>
#include <sstream>
#include <utility>
#include <vector>

namespace tempolink::net {
namespace {

#ifdef _WIN32
using SocketLen = int;
using NativeSocket = SOCKET;
constexpr NativeSocket kInvalidNativeSocket = INVALID_SOCKET;
#else
using SocketLen = socklen_t;
using NativeSocket = int;
constexpr NativeSocket kInvalidNativeSocket = -1;
#endif

NativeSocket ToNative(std::intptr_t socket) {
  return static_cast<NativeSocket>(socket);
}

std::intptr_t ToPortable(NativeSocket socket) {
  return static_cast<std::intptr_t>(socket);
}

void CloseSocket(NativeSocket socket) {
#ifdef _WIN32
  closesocket(socket);
#else
  close(socket);
#endif
}

bool StartupSockets() {
#ifdef _WIN32
  static std::once_flag once;
  static bool initialized = false;
  std::call_once(once, [] {
    WSADATA data{};
    initialized = (WSAStartup(MAKEWORD(2, 2), &data) == 0);
  });
  return initialized;
#else
  return true;
#endif
}

bool IsWouldBlockError() {
#ifdef _WIN32
  const int error = WSAGetLastError();
  return error == WSAEWOULDBLOCK;
#else
  return errno == EWOULDBLOCK || errno == EAGAIN;
#endif
}

}  // namespace

UdpSocket::UdpSocket() = default;

UdpSocket::~UdpSocket() { Close(); }

UdpSocket::UdpSocket(UdpSocket&& other) noexcept : socket_(other.socket_) {
  other.socket_ = kInvalidSocket;
}

UdpSocket& UdpSocket::operator=(UdpSocket&& other) noexcept {
  if (this == &other) {
    return *this;
  }
  Close();
  socket_ = other.socket_;
  other.socket_ = kInvalidSocket;
  return *this;
}

bool UdpSocket::Open() {
  if (IsOpen()) {
    return true;
  }

  if (!StartupSockets()) {
    return false;
  }

  const NativeSocket sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sock == kInvalidNativeSocket) {
    return false;
  }

  socket_ = ToPortable(sock);
  return true;
}

bool UdpSocket::Bind(std::uint16_t port, const std::string& bind_address) {
  if (!IsOpen() && !Open()) {
    return false;
  }

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  if (bind_address == "0.0.0.0") {
    addr.sin_addr.s_addr = INADDR_ANY;
  } else {
    if (inet_pton(AF_INET, bind_address.c_str(), &addr.sin_addr) != 1) {
      return false;
    }
  }

  const int reuse = 1;
  setsockopt(ToNative(socket_), SOL_SOCKET, SO_REUSEADDR,
             reinterpret_cast<const char*>(&reuse), sizeof(reuse));

  const int result = bind(ToNative(socket_), reinterpret_cast<sockaddr*>(&addr),
                          sizeof(addr));
  return result == 0;
}

bool UdpSocket::SetNonBlocking(bool enabled) {
  if (!IsOpen()) {
    return false;
  }

#ifdef _WIN32
  u_long mode = enabled ? 1UL : 0UL;
  return ioctlsocket(ToNative(socket_), FIONBIO, &mode) == 0;
#else
  int flags = fcntl(ToNative(socket_), F_GETFL, 0);
  if (flags < 0) {
    return false;
  }
  if (enabled) {
    flags |= O_NONBLOCK;
  } else {
    flags &= ~O_NONBLOCK;
  }
  return fcntl(ToNative(socket_), F_SETFL, flags) == 0;
#endif
}

bool UdpSocket::SendTo(std::span<const std::byte> data, const std::string& host,
                       std::uint16_t port) {
  if (!IsOpen()) {
    return false;
  }

  addrinfo hints{};
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_protocol = IPPROTO_UDP;

  addrinfo* result = nullptr;
  const std::string port_text = std::to_string(port);
  if (getaddrinfo(host.c_str(), port_text.c_str(), &hints, &result) != 0 ||
      result == nullptr) {
    return false;
  }

  const int sent = sendto(
      ToNative(socket_), reinterpret_cast<const char*>(data.data()),
      static_cast<int>(data.size()), 0, result->ai_addr,
      static_cast<int>(result->ai_addrlen));
  freeaddrinfo(result);
  return sent >= 0;
}

std::optional<Datagram> UdpSocket::ReceiveFrom(std::size_t max_size) {
  if (!IsOpen()) {
    return std::nullopt;
  }

  std::vector<std::byte> buffer(max_size);
  sockaddr_in source{};
  SocketLen source_size = sizeof(source);
  const int read = recvfrom(
      ToNative(socket_), reinterpret_cast<char*>(buffer.data()),
      static_cast<int>(buffer.size()), 0, reinterpret_cast<sockaddr*>(&source),
      &source_size);
  if (read < 0) {
    if (IsWouldBlockError()) {
      return std::nullopt;
    }
    return std::nullopt;
  }

  buffer.resize(static_cast<std::size_t>(read));
  char host_buf[INET_ADDRSTRLEN] = {0};
  inet_ntop(AF_INET, &source.sin_addr, host_buf, sizeof(host_buf));

  Datagram datagram;
  datagram.data = std::move(buffer);
  datagram.host = host_buf;
  datagram.port = ntohs(source.sin_port);
  return datagram;
}

bool UdpSocket::IsOpen() const {
  return ToNative(socket_) != kInvalidNativeSocket;
}

void UdpSocket::Close() {
  if (!IsOpen()) {
    return;
  }
  CloseSocket(ToNative(socket_));
  socket_ = kInvalidSocket;
}

std::string UdpSocket::LastErrorMessage() {
#ifdef _WIN32
  return "WSA error: " + std::to_string(WSAGetLastError());
#else
  return std::strerror(errno);
#endif
}

}  // namespace tempolink::net
