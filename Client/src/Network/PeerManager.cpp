#include <tempolink/network/PeerManager.h>

void PeerManager::addPeer(const std::string& id, const asio::ip::udp::endpoint& endpoint)
{
    peers[id] = Peer { id, endpoint };
}

void PeerManager::removePeer(const std::string& id)
{
    peers.erase(id);
}

const Peer* PeerManager::getPeer(const std::string& id) const
{
    const auto it = peers.find(id);
    return it != peers.end() ? &it->second : nullptr;
}
