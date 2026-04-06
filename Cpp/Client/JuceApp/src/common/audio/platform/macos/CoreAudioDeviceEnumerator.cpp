#include "tempolink/audio/platform/macos/CoreAudioDeviceEnumerator.h"

#include <CoreAudio/CoreAudio.h>
#include <CoreFoundation/CoreFoundation.h>

#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace tempolink::audio::platform::macos::detail {
namespace {

std::string CFStringToUtf8(CFStringRef cf_string) {
  if (cf_string == nullptr) {
    return {};
  }

  const CFIndex length = CFStringGetLength(cf_string);
  if (length <= 0) {
    return {};
  }

  const CFIndex max_size =
      CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8) + 1;
  std::string utf8(static_cast<std::size_t>(max_size), '\0');
  if (!CFStringGetCString(cf_string, utf8.data(), max_size, kCFStringEncodingUTF8)) {
    return {};
  }
  utf8.resize(std::strlen(utf8.c_str()));
  return utf8;
}

bool DeviceSupportsDirection(AudioDeviceID device_id, bool input) {
  AudioObjectPropertyAddress config_addr{};
  config_addr.mSelector = kAudioDevicePropertyStreamConfiguration;
  config_addr.mScope =
      input ? kAudioDevicePropertyScopeInput : kAudioDevicePropertyScopeOutput;
  config_addr.mElement = kAudioObjectPropertyElementMain;

  UInt32 data_size = 0;
  if (AudioObjectGetPropertyDataSize(device_id, &config_addr, 0, nullptr,
                                     &data_size) != noErr ||
      data_size == 0) {
    return false;
  }

  auto* buffer_list = static_cast<AudioBufferList*>(std::malloc(data_size));
  if (buffer_list == nullptr) {
    return false;
  }

  auto free_buffer = [](AudioBufferList* p) { if (p) std::free(p); };
  std::unique_ptr<AudioBufferList, decltype(free_buffer)> guard(buffer_list, free_buffer);

  if (AudioObjectGetPropertyData(device_id, &config_addr, 0, nullptr, &data_size,
                                 buffer_list) != noErr) {
    return false;
  }

  for (UInt32 i = 0; i < buffer_list->mNumberBuffers; ++i) {
    if (buffer_list->mBuffers[i].mNumberChannels > 0) {
      return true;
    }
  }
  return false;
}

std::string DeviceName(AudioDeviceID device_id) {
  AudioObjectPropertyAddress name_addr{};
  name_addr.mSelector = kAudioObjectPropertyName;
  name_addr.mScope = kAudioObjectPropertyScopeGlobal;
  name_addr.mElement = kAudioObjectPropertyElementMain;

  CFStringRef name_ref = nullptr;
  UInt32 size = static_cast<UInt32>(sizeof(name_ref));
  if (AudioObjectGetPropertyData(device_id, &name_addr, 0, nullptr, &size,
                                 &name_ref) != noErr ||
      name_ref == nullptr) {
    return {};
  }

  const std::string name = CFStringToUtf8(name_ref);
  CFRelease(name_ref);
  return name;
}

AudioDeviceID DefaultDevice(bool input) {
  AudioObjectPropertyAddress addr{};
  addr.mSelector = input ? kAudioHardwarePropertyDefaultInputDevice
                         : kAudioHardwarePropertyDefaultOutputDevice;
  addr.mScope = kAudioObjectPropertyScopeGlobal;
  addr.mElement = kAudioObjectPropertyElementMain;

  AudioDeviceID device_id = kAudioObjectUnknown;
  UInt32 size = static_cast<UInt32>(sizeof(device_id));
  if (AudioObjectGetPropertyData(kAudioObjectSystemObject, &addr, 0, nullptr,
                                 &size, &device_id) != noErr) {
    return kAudioObjectUnknown;
  }
  return device_id;
}

std::vector<AudioDeviceInfo> EnumerateDevices(bool input) {
  std::vector<AudioDeviceInfo> devices;

  AudioObjectPropertyAddress addr{};
  addr.mSelector = kAudioHardwarePropertyDevices;
  addr.mScope = kAudioObjectPropertyScopeGlobal;
  addr.mElement = kAudioObjectPropertyElementMain;

  UInt32 size = 0;
  if (AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &addr, 0, nullptr,
                                     &size) != noErr ||
      size == 0) {
    return devices;
  }

  const UInt32 count = size / static_cast<UInt32>(sizeof(AudioDeviceID));
  std::vector<AudioDeviceID> ids(count, kAudioObjectUnknown);
  if (AudioObjectGetPropertyData(kAudioObjectSystemObject, &addr, 0, nullptr, &size,
                                 ids.data()) != noErr) {
    return devices;
  }

  const AudioDeviceID default_id = DefaultDevice(input);
  devices.reserve(ids.size());

  for (const auto device_id : ids) {
    if (device_id == kAudioObjectUnknown || !DeviceSupportsDirection(device_id, input)) {
      continue;
    }

    AudioDeviceInfo info;
    info.id = std::to_string(device_id);
    info.name = DeviceName(device_id);
    if (info.name.empty()) {
      info.name = info.id;
    }
    info.is_default = (device_id == default_id);
    devices.push_back(std::move(info));
  }

  return devices;
}

}  // namespace

std::vector<AudioDeviceInfo> EnumerateCoreAudioInputDevices() {
  return EnumerateDevices(true);
}

std::vector<AudioDeviceInfo> EnumerateCoreAudioOutputDevices() {
  return EnumerateDevices(false);
}

}  // namespace tempolink::audio::platform::macos::detail
