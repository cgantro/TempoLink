#pragma once
#include <vector>
#include <deque>
#include <cstdint>

class JitterBuffer
{
public:
    explicit JitterBuffer(int maxPackets = 8);

    void push(uint32_t sequence, std::vector<uint8_t> packet);
    bool pop(std::vector<uint8_t>& outPacket);
    void reset();

private:
    struct Entry
    {
        uint32_t sequence;
        std::vector<uint8_t> data;
    };

    std::deque<Entry> buffer;
    int maxPackets;
    uint32_t nextExpectedSeq = 0;
};
