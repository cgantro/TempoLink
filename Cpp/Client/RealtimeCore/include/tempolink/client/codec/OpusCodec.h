#pragma once

#ifdef TEMPOLINK_USE_OPUS

#include <opus.h>
#include <vector>
#include <cstdint>

namespace tempolink::client::codec {

class OpusAudioEncoder {
public:
    // 샘플레이트(보통 48000), 채널 수(1 or 2), 애플리케이션 타입(OPUS_APPLICATION_AUDIO 또는 OPUS_APPLICATION_VOIP)
    OpusAudioEncoder(int sampleRate, int channels, int application = OPUS_APPLICATION_AUDIO);
    ~OpusAudioEncoder();

    // PCM 데이터를 Opus 패킷으로 인코딩합니다. 성공 시 인코딩된 바이트 수를 반환합니다.
    int encode(const float* pcm, int frameSize, std::vector<uint8_t>& outPacket);

    // 동적 비트레이트 조절 (예: 64000 -> 64kbps, 기본값: OPUS_AUTO)
    bool setBitrate(int bitrate);

    // 동적 복잡도 조절 (0 ~ 10, 숫자가 낮을수록 CPU 점유율 감소, 기본값: 9)
    bool setComplexity(int complexity);

private:
    OpusEncoder* encoder_{nullptr};
    int channels_{1};
};

class OpusAudioDecoder {
public:
    OpusAudioDecoder(int sampleRate, int channels);
    ~OpusAudioDecoder();

    // Opus 패킷을 PCM 데이터로 디코딩합니다. (네트워크 손실 복구 시 decodeFec를 1로 설정)
    // 성공 시 디코딩된 샘플 수(프레임 크기)를 반환합니다.
    int decode(const uint8_t* packet, int packetLen, std::vector<float>& outPcm, int frameSize, int decodeFec = 0);

private:
    OpusDecoder* decoder_{nullptr};
    int channels_{1};
};

} // namespace tempolink::client::codec

#endif // TEMPOLINK_USE_OPUS