#include <tempolink/network/UdpSession.h>

#include <memory>
#include <utility>

UdpSession::UdpSession(uint16_t localPort, ReceiveCallback onReceiveCallback)
    : socket(ioContext, asio::ip::udp::endpoint(asio::ip::udp::v4(), localPort)),
      onReceive(std::move(onReceiveCallback))
{
}

UdpSession::~UdpSession()
{
    stop();
}

void UdpSession::send(const asio::ip::udp::endpoint& remote, const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0)
        return;

    auto payload = std::make_shared<std::vector<uint8_t>>(data, data + size);
    socket.async_send_to(asio::buffer(*payload), remote, [payload](const std::error_code&, std::size_t) {});
}

void UdpSession::start()
{
    if (ioThread.joinable())
        return;

    ioContext.restart();
    doReceive();
    ioThread = std::thread([this] { ioContext.run(); });
}

void UdpSession::stop()
{
    ioContext.stop();

    std::error_code ignored;
    socket.close(ignored);

    if (ioThread.joinable())
        ioThread.join();
}

void UdpSession::doReceive()
{
    socket.async_receive_from(
        asio::buffer(recvBuffer),
        senderEndpoint,
        [this](const std::error_code& error, std::size_t bytesReceived)
        {
            if (!error && bytesReceived > 0 && onReceive)
                onReceive(recvBuffer.data(), bytesReceived);

            if (!error && socket.is_open())
                doReceive();
        });
}
