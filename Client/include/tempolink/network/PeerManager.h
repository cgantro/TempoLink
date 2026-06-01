#pragma once
#include <asio.hpp>
#include <unordered_map>
#include <string>

struct Peer
{
    std::string             id;
    asio::ip::udp::endpoint endpoint;
};

class PeerManager
{
public:
    void addPeer(const std::string& id, const asio::ip::udp::endpoint& endpoint);
    void removePeer(const std::string& id);
    const Peer* getPeer(const std::string& id) const;

private:
    std::unordered_map<std::string, Peer> peers;
};
