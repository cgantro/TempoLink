#include <tempolink/audio/JitterBuffer.h>

#include <algorithm>

JitterBuffer::JitterBuffer(int maxPacketsToKeep)
    : maxPackets(maxPacketsToKeep)
{
}

void JitterBuffer::push(uint32_t sequence, std::vector<uint8_t> packet)
{
    auto position = std::find_if(buffer.begin(), buffer.end(), [sequence](const Entry& entry)
    {
        return entry.sequence > sequence;
    });

    buffer.insert(position, Entry { sequence, std::move(packet) });

    while (static_cast<int>(buffer.size()) > maxPackets)
        buffer.pop_front();
}

bool JitterBuffer::pop(std::vector<uint8_t>& outPacket)
{
    if (buffer.empty())
        return false;

    auto entry = std::move(buffer.front());
    buffer.pop_front();
    nextExpectedSeq = entry.sequence + 1;
    outPacket = std::move(entry.data);
    return true;
}

void JitterBuffer::reset()
{
    buffer.clear();
    nextExpectedSeq = 0;
}
