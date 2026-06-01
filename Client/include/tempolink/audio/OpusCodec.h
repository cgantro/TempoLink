#pragma once
#include <vector>
#include <cstdint>

class OpusCodec
{
public:
    OpusCodec(int sampleRate, int channels);
    ~OpusCodec();

    std::vector<uint8_t> encode(const float* pcm, int frameSize);
    int decode(const uint8_t* data, int dataSize, float* pcm, int maxFrameSize);

private:
    struct Impl;
    Impl* impl = nullptr;
};
