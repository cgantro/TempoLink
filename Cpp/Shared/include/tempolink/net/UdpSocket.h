#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <string>

namespace tempolink::net {

enum class SocketStatus : std::uint8_t {
  Success = 0,
  WouldBlock,
  Closed,
  Error,
};

struct Endpoint {
  // IPv4 address in host byte order.
  std::uint32_t address_ipv4 = 0;
  std::uint16_t port = 0;
};

struct ReceiveResult {
  SocketStatus status = SocketStatus::Error;
  std::size_t bytes_read = 0;
  Endpoint sender{};
};

/// Cross-platform UDP socket. Platform details are hidden behind Pimpl.
class UdpSocket {
 public:
  UdpSocket();
  ~UdpSocket();

  UdpSocket(const UdpSocket&) = delete;
  UdpSocket& operator=(const UdpSocket&) = delete;
  UdpSocket(UdpSocket&& other) noexcept;
  UdpSocket& operator=(UdpSocket&& other) noexcept;

  bool Open();
  bool Bind(std::uint16_t port, const std::string& bind_address = "0.0.0.0");
  bool SetNonBlocking(bool enabled);

  bool SendTo(std::span<const std::byte> data, const std::string& host,
              std::uint16_t port);
  ReceiveResult ReceiveFrom(std::span<std::byte> buffer);

  bool IsOpen() const;
  void Close();

  static std::string LastErrorMessage();

 private:
  /// Pimpl — platform-specific implementation.
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace tempolink::net
