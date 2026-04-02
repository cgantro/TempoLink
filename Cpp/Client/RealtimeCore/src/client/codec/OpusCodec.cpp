#ifdef TEMPOLINK_USE_OPUS

#include "tempolink/client/codec/OpusCodec.h"
#include <stdexcept>
#include <string>

namespace tempolink::client::codec {

OpusAudioEncoder::OpusAudioEncoder(int sampleRate, int channels, int application)
    : channels_(channels) {
    int err = OPUS_OK;
    encoder_ = opus_encoder_create(sampleRate, channels, application, &err);
    if (err != OPUS_OK) {
        throw std::runtime_error("Failed to create Opus encoder: error code " + std::to_string(err));
    }
}

OpusAudioEncoder::~OpusAudioEncoder() {
    if (encoder_) {
        opus_encoder_destroy(encoder_);
    }
}

int OpusAudioEncoder::encode(const float* pcm, int frameSize, std::vector<uint8_t>& outPacket) {
    // 예약된 용량이 충분한지 확인하여 불필요한 재할당을 방지합니다.
    if (outPacket.capacity() < 4000) {
        outPacket.reserve(4000);
    }
    // 메모리가 0으로 초기화되는 오버헤드가 발생하지만 크기 설정을 위해 필요합니다.
    outPacket.resize(4000);
    
    int bytes = opus_encode_float(encoder_, pcm, frameSize, outPacket.data(), static_cast<int>(outPacket.size()));
    if (bytes < 0) {
        outPacket.clear();
        return bytes; // 음수이면 에러 코드
    }
    outPacket.resize(bytes); // 실제 인코딩된 바이트 수로 축소 (이때는 초기화 오버헤드가 없음)
    return bytes;
}

bool OpusAudioEncoder::setBitrate(int bitrate) {
    if (!encoder_) return false;
    // OPUS_SET_BITRATE 매크로를 통해 즉시 비트레이트를 변경합니다.
    return opus_encoder_ctl(encoder_, OPUS_SET_BITRATE(bitrate)) == OPUS_OK;
}

bool OpusAudioEncoder::setComplexity(int complexity) {
    if (!encoder_) return false;
    // complexity는 0부터 10까지의 범위로 제한됩니다.
    if (complexity < 0) complexity = 0;
    if (complexity > 10) complexity = 10;
    
    return opus_encoder_ctl(encoder_, OPUS_SET_COMPLEXITY(complexity)) == OPUS_OK;
}

OpusAudioDecoder::OpusAudioDecoder(int sampleRate, int channels)
    : channels_(channels) {
    int err = OPUS_OK;
    decoder_ = opus_decoder_create(sampleRate, channels, &err);
    if (err != OPUS_OK) {
        throw std::runtime_error("Failed to create Opus decoder: error code " + std::to_string(err));
    }
}

OpusAudioDecoder::~OpusAudioDecoder() {
    if (decoder_) {
        opus_decoder_destroy(decoder_);
    }
}

int OpusAudioDecoder::decode(const uint8_t* packet, int packetLen, std::vector<float>& outPcm, int frameSize, int decodeFec) {
    const int requiredSize = frameSize * channels_;
    if (outPcm.capacity() < requiredSize) {
        outPcm.reserve(requiredSize);
    }
    
    outPcm.resize(frameSize * channels_);
    int samples = opus_decode_float(decoder_, packet, packetLen, outPcm.data(), frameSize, decodeFec);
    if (samples < 0) {
        outPcm.clear();
        return samples; // 음수이면 에러 코드
    }
    return samples;
}

} // namespace tempolink::client::codec
#endif // TEMPOLINK_USE_OPUS