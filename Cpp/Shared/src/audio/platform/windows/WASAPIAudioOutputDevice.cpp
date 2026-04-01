#include "tempolink/audio/platform/windows/WASAPIAudioOutputDevice.h"

#include <algorithm>
#include <chrono>
#include <windows.h>
#include <audioclient.h>
#include <mmdeviceapi.h>
#include <objbase.h>
#include <thread>
#include <utility>

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

IMMDevice* ResolveRenderDevice(const std::string& device_id) {
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
    enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);
  }
  enumerator->Release();
  return device;
}

}  // namespace

std::vector<AudioDeviceInfo> WASAPIAudioOutputDevice::ListDevices() const {
  return platform::windows::detail::EnumerateWasapiOutputDevices();
}

bool WASAPIAudioOutputDevice::SelectDevice(const std::string& device_id) {
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

std::string WASAPIAudioOutputDevice::SelectedDeviceId() const {
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

bool WASAPIAudioOutputDevice::Start(const AudioPlaybackConfig& config) {
  Stop();
  config_ = config;
  running_.store(true);
  render_thread_ = std::jthread([this](std::stop_token token) { RenderLoop(token); });
  return true;
}

void WASAPIAudioOutputDevice::Stop() {
  if (!running_.exchange(false)) {
    return;
  }
  if (render_thread_.joinable()) {
    render_thread_.request_stop();
    render_thread_.join();
  }
  std::scoped_lock lock(queue_mutex_);
  pending_samples_.clear();
}

bool WASAPIAudioOutputDevice::IsRunning() const { return running_.load(); }

std::string WASAPIAudioOutputDevice::BackendName() const { return "WASAPI"; }

void WASAPIAudioOutputDevice::SetOutputVolume(float gain) {
  output_volume_.store(std::clamp(gain, 0.0F, 1.0F));
}

float WASAPIAudioOutputDevice::OutputVolume() const {
  return output_volume_.load();
}

void WASAPIAudioOutputDevice::PlayFrame(std::span<const std::int16_t> pcm) {
  if (!running_.load()) {
    return;
  }
  std::scoped_lock lock(queue_mutex_);
  pending_samples_.insert(pending_samples_.end(), pcm.begin(), pcm.end());
  constexpr std::size_t kMaxBufferedSamples = 48000 * 4;
  while (pending_samples_.size() > kMaxBufferedSamples) {
    pending_samples_.pop_front();
  }
}

void WASAPIAudioOutputDevice::RenderLoop(std::stop_token stop_token) {
  const HRESULT com_hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
  const bool should_uninit = SUCCEEDED(com_hr);
  if (FAILED(com_hr) && com_hr != RPC_E_CHANGED_MODE) {
    running_.store(false);
    return;
  }

  IMMDevice* device = ResolveRenderDevice(selected_device_id_);
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
  fmt.nChannels = static_cast<WORD>(std::max<std::uint8_t>(1, config_.channels));
  fmt.nSamplesPerSec = std::max<std::uint32_t>(8000, config_.sample_rate_hz);
  fmt.wBitsPerSample = 16;
  fmt.nBlockAlign = static_cast<WORD>(fmt.nChannels * (fmt.wBitsPerSample / 8));
  fmt.nAvgBytesPerSec = fmt.nSamplesPerSec * fmt.nBlockAlign;
  fmt.cbSize = 0;

  const REFERENCE_TIME hns_buffer = static_cast<REFERENCE_TIME>(
      (10000000LL * std::max<std::uint16_t>(64, config_.frame_samples) * 4ULL) /
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

  IAudioRenderClient* render_client = nullptr;
  hr = audio_client->GetService(__uuidof(IAudioRenderClient),
                                reinterpret_cast<void**>(&render_client));
  if (FAILED(hr) || render_client == nullptr) {
    audio_client->Release();
    device->Release();
    if (should_uninit) {
      CoUninitialize();
    }
    running_.store(false);
    return;
  }

  UINT32 buffer_frames = 0;
  hr = audio_client->GetBufferSize(&buffer_frames);
  if (FAILED(hr) || buffer_frames == 0) {
    render_client->Release();
    audio_client->Release();
    device->Release();
    if (should_uninit) {
      CoUninitialize();
    }
    running_.store(false);
    return;
  }

  BYTE* init_data = nullptr;
  hr = render_client->GetBuffer(buffer_frames, &init_data);
  if (SUCCEEDED(hr) && init_data != nullptr) {
    const std::size_t init_samples =
        static_cast<std::size_t>(buffer_frames) * static_cast<std::size_t>(fmt.nChannels);
    std::fill_n(reinterpret_cast<std::int16_t*>(init_data), init_samples, 0);
    render_client->ReleaseBuffer(buffer_frames, 0);
  }

  if (FAILED(audio_client->Start())) {
    render_client->Release();
    audio_client->Release();
    device->Release();
    if (should_uninit) {
      CoUninitialize();
    }
    running_.store(false);
    return;
  }

  while (!stop_token.stop_requested() && running_.load()) {
    UINT32 padding = 0;
    if (FAILED(audio_client->GetCurrentPadding(&padding))) {
      break;
    }
    const UINT32 available_frames =
        (padding >= buffer_frames) ? 0 : (buffer_frames - padding);
    if (available_frames == 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(2));
      continue;
    }

    BYTE* buffer_data = nullptr;
    hr = render_client->GetBuffer(available_frames, &buffer_data);
    if (FAILED(hr) || buffer_data == nullptr) {
      std::this_thread::sleep_for(std::chrono::milliseconds(2));
      continue;
    }

    auto* out = reinterpret_cast<std::int16_t*>(buffer_data);
    const std::size_t needed_samples =
        static_cast<std::size_t>(available_frames) * static_cast<std::size_t>(fmt.nChannels);
    const float gain = output_volume_.load();

    std::size_t consumed = 0;
    {
      std::scoped_lock lock(queue_mutex_);
      while (consumed < needed_samples && !pending_samples_.empty()) {
        const float scaled =
            static_cast<float>(pending_samples_.front()) * std::clamp(gain, 0.0F, 1.0F);
        out[consumed++] =
            static_cast<std::int16_t>(std::clamp(scaled, -32768.0F, 32767.0F));
        pending_samples_.pop_front();
      }
    }
    while (consumed < needed_samples) {
      out[consumed++] = 0;
    }

    render_client->ReleaseBuffer(available_frames, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  audio_client->Stop();
  render_client->Release();
  audio_client->Release();
  device->Release();
  if (should_uninit) {
    CoUninitialize();
  }
}

}  // namespace tempolink::audio
