#include "tempolink/net/UdpSocket.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <span>
#include <utility>

namespace tempolink::net {

// -----------------------------------------------------------------------
// Pimpl — POSIX (Linux / macOS) implementation
// -----------------------------------------------------------------------

struct UdpSocket::Impl {
  int socket = -1;

  bool IsOpen() const { return socket >= 0; }

  void Close() {
    if (IsOpen()) {
      ::close(socket);
      socket = -1;
    }
  }

  bool Open() {
    if (IsOpen()) return true;
    socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    return socket >= 0;
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
    setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    return ::bind(socket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == 0;
  }

  bool SetNonBlocking(bool enabled) {
    if (!IsOpen()) return false;
    int flags = fcntl(socket, F_GETFL, 0);
    if (flags < 0) return false;
    if (enabled) {
      flags |= O_NONBLOCK;
    } else {
      flags &= ~O_NONBLOCK;
    }
    return fcntl(socket, F_SETFL, flags) == 0;
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
    const ssize_t sent =
        sendto(socket, reinterpret_cast<const char*>(data.data()),
               data.size(), 0, result->ai_addr, result->ai_addrlen);
    freeaddrinfo(result);
    return sent >= 0;
  }

  ReceiveResult ReceiveFrom(std::span<std::byte> buffer) {
    if (!IsOpen()) return {SocketStatus::Closed, 0, {}};
    if (buffer.empty()) return {SocketStatus::Error, 0, {}};
    sockaddr_in source{};
    socklen_t source_size = sizeof(source);
    const ssize_t read = recvfrom(
        socket, reinterpret_cast<char*>(buffer.data()), buffer.size(), 0,
        reinterpret_cast<sockaddr*>(&source), &source_size);
    if (read < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        return {SocketStatus::WouldBlock, 0, {}};
      }
      if (errno == EBADF || errno == ENOTSOCK) {
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

  static std::string LastErrorMessage() { return std::strerror(errno); }
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
