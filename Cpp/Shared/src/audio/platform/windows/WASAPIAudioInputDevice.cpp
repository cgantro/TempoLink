#include "tempolink/audio/platform/windows/WASAPIAudioInputDevice.h"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <windows.h>
#include <audioclient.h>
#include <mmdeviceapi.h>
#include <objbase.h>
#include <thread>
#include <utility>
#include <vector>

#include "tempolink/audio/platform/windows/WASAPIDeviceEnumerator.h"

namespace tempolink::audio {
namespace {

std::wstring Utf8ToWide(const std::string& utf8) {
  if (utf8.empty()) {
    return {};
  }
  const int required = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
  if (required <= 1) {
    return {};
  }
  std::wstring wide(static_cast<std::size_t>(required - 1), L'\0');
  MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, wide.data(), required);
  return wide;
}

IMMDevice* ResolveCaptureDevice(const std::string& device_id) {
  IMMDeviceEnumerator* enumerator = nullptr;
  if (FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                              __uuidof(IMMDeviceEnumerator),
                              reinterpret_cast<void**>(&enumerator))) ||
      enumerator == nullptr) {
    return nullptr;
  }

  IMMDevice* device = nullptr;
  if (!device_id.empty()) {
    const std::wstring wide_id = Utf8ToWide(device_id);
    if (!wide_id.empty()) {
      enumerator->GetDevice(wide_id.c_str(), &device);
    }
  }
  if (device == nullptr) {
    enumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &device);
  }

  enumerator->Release();
  return device;
}

}  // namespace

std::vector<AudioDeviceInfo> WASAPIAudioInputDevice::ListDevices() const {
  return platform::windows::detail::EnumerateWasapiInputDevices();
}

bool WASAPIAudioInputDevice::SelectDevice(const std::string& device_id) {
  const auto devices = ListDevices();
  for (const auto& device : devices) {
    if (device.id == device_id || device.name == device_id) {
      selected_device_id_ = device_id;
      if (device.id != device_id) {
        selected_device_id_ = device.id;
      }
      return true;
    }
  }
  return false;
}

std::string WASAPIAudioInputDevice::SelectedDeviceId() const {
  const auto devices = ListDevices();
  for (const auto& device : devices) {
    if (device.id == selected_device_id_) {
      return selected_device_id_;
    }
  }
  for (const auto& device : devices) {
    if (device.is_default) {
      return device.id;
    }
  }
  if (!devices.empty()) {
    return devices.front().id;
  }
  return selected_device_id_;
}

bool WASAPIAudioInputDevice::Start(const AudioCaptureConfig& config,
                                   AudioCaptureCallback callback) {
  if (running_.exchange(true)) {
    return false;
  }
  config_ = config;
  capture_thread_ = std::jthread(
      [this, config, callback = std::move(callback)](std::stop_token token) {
        CaptureLoop(token, config, callback);
      });
  return true;
}

void WASAPIAudioInputDevice::Stop() {
  if (!running_.exchange(false)) {
    return;
  }
  if (capture_thread_.joinable()) {
    capture_thread_.request_stop();
    capture_thread_.join();
  }
}

bool WASAPIAudioInputDevice::IsRunning() const { return running_.load(); }

std::string WASAPIAudioInputDevice::BackendName() const { return "WASAPI"; }

void WASAPIAudioInputDevice::CaptureLoop(std::stop_token stop_token,
                                         AudioCaptureConfig config,
                                         AudioCaptureCallback callback) {
  if (!callback) {
    running_.store(false);
    return;
  }

  const HRESULT com_hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
  const bool should_uninit = SUCCEEDED(com_hr);
  if (FAILED(com_hr) && com_hr != RPC_E_CHANGED_MODE) {
    running_.store(false);
    return;
  }

  IMMDevice* device = ResolveCaptureDevice(selected_device_id_);
  if (device == nullptr) {
    if (should_uninit) {
      CoUninitialize();
    }
    running_.store(false);
    return;
  }

  IAudioClient* audio_client = nullptr;
  HRESULT hr = device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr,
                                reinterpret_cast<void**>(&audio_client));
  if (FAILED(hr) || audio_client == nullptr) {
    device->Release();
    if (should_uninit) {
      CoUninitialize();
    }
    running_.store(false);
    return;
  }

  WAVEFORMATEX fmt{};
  fmt.wFormatTag = WAVE_FORMAT_PCM;
  fmt.nChannels = static_cast<WORD>(std::max<std::uint8_t>(1, config.channels));
  fmt.nSamplesPerSec = std::max<std::uint32_t>(8000, config.sample_rate_hz);
  fmt.wBitsPerSample = 16;
  fmt.nBlockAlign = static_cast<WORD>(fmt.nChannels * (fmt.wBitsPerSample / 8));
  fmt.nAvgBytesPerSec = fmt.nSamplesPerSec * fmt.nBlockAlign;
  fmt.cbSize = 0;

  const REFERENCE_TIME hns_buffer = static_cast<REFERENCE_TIME>(
      (10000000LL * std::max<std::uint16_t>(64, config.frame_samples) * 4ULL) /
      fmt.nSamplesPerSec);
  constexpr DWORD kFlags =
      AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY;
  hr = audio_client->Initialize(AUDCLNT_SHAREMODE_SHARED, kFlags, hns_buffer, 0,
                                &fmt, nullptr);
  if (FAILED(hr)) {
    audio_client->Release();
    device->Release();
    if (should_uninit) {
      CoUninitialize();
    }
    running_.store(false);
    return;
  }

  IAudioCaptureClient* capture_client = nullptr;
  hr = audio_client->GetService(__uuidof(IAudioCaptureClient),
                                reinterpret_cast<void**>(&capture_client));
  if (FAILED(hr) || capture_client == nullptr) {
    audio_client->Release();
    device->Release();
    if (should_uninit) {
      CoUninitialize();
    }
    running_.store(false);
    return;
  }

  if (FAILED(audio_client->Start())) {
    capture_client->Release();
    audio_client->Release();
    device->Release();
    if (should_uninit) {
      CoUninitialize();
    }
    running_.store(false);
    return;
  }

  const std::size_t chunk_samples =
      static_cast<std::size_t>(std::max<std::uint16_t>(1, config.frame_samples)) *
      static_cast<std::size_t>(fmt.nChannels);
  std::vector<std::int16_t> pending;
  pending.reserve(chunk_samples * 3);

  while (!stop_token.stop_requested() && running_.load()) {
    UINT32 packet_frames = 0;
    hr = capture_client->GetNextPacketSize(&packet_frames);
    if (FAILED(hr)) {
      break;
    }

    if (packet_frames == 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(2));
      continue;
    }

    while (packet_frames > 0) {
      BYTE* data = nullptr;
      UINT32 frames = 0;
      DWORD flags = 0;
      hr = capture_client->GetBuffer(&data, &frames, &flags, nullptr, nullptr);
      if (FAILED(hr)) {
        packet_frames = 0;
        break;
      }

      const std::size_t samples =
          static_cast<std::size_t>(frames) * static_cast<std::size_t>(fmt.nChannels);
      if ((flags & AUDCLNT_BUFFERFLAGS_SILENT) != 0 || data == nullptr) {
        pending.insert(pending.end(), samples, 0);
      } else {
        const auto* source = reinterpret_cast<const std::int16_t*>(data);
        pending.insert(pending.end(), source, source + samples);
      }

      capture_client->ReleaseBuffer(frames);

      while (pending.size() >= chunk_samples) {
        callback(std::span<const std::int16_t>(pending.data(), chunk_samples));
        pending.erase(pending.begin(), pending.begin() + static_cast<std::ptrdiff_t>(chunk_samples));
      }

      hr = capture_client->GetNextPacketSize(&packet_frames);
      if (FAILED(hr)) {
        packet_frames = 0;
        break;
      }
    }
  }

  audio_client->Stop();
  capture_client->Release();
  audio_client->Release();
  device->Release();
  if (should_uninit) {
    CoUninitialize();
  }
}

}  // namespace tempolink::audio
