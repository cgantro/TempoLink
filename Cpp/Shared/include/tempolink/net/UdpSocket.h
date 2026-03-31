#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace tempolink::net {

struct Datagram {
  std::vector<std::byte> data;
  std::string host;
  std::uint16_t port = 0;
};

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
  std::optional<Datagram> ReceiveFrom(std::size_t max_size = 2048);

  bool IsOpen() const;
  void Close();

  static std::string LastErrorMessage();

 private:
  using NativeSocket = std::intptr_t;
  static constexpr NativeSocket kInvalidSocket = -1;

  NativeSocket socket_ = kInvalidSocket;
};

}  // namespace tempolink::net

