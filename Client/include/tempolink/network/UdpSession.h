#pragma once
#include <asio.hpp>
#include <functional>
#include <vector>
#include <thread>
#include <array>
#include <cstdint>

class UdpSession
{
public:
    using ReceiveCallback = std::function<void(const uint8_t*, size_t)>;

    UdpSession(uint16_t localPort, ReceiveCallback onReceive);
    ~UdpSession();

    void send(const asio::ip::udp::endpoint& remote, const uint8_t* data, size_t size);
    void start();
    void stop();

private:
    void doReceive();

    asio::io_context           ioContext;
    asio::ip::udp::socket      socket;
    asio::ip::udp::endpoint    senderEndpoint;
    std::array<uint8_t, 4096>  recvBuffer;
    ReceiveCallback            onReceive;
    std::thread                ioThread;
};
