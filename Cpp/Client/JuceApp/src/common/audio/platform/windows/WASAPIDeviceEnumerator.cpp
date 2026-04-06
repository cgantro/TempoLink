#include "tempolink/audio/platform/windows/WASAPIDeviceEnumerator.h"

#include <windows.h>
#include <mmdeviceapi.h>
#include <objbase.h>
#include <propidl.h>
#include <propvarutil.h>

#include <cstddef>
#include <string>
#include <utility>

namespace tempolink::audio::platform::windows::detail {
namespace {

// Equivalent to PKEY_Device_FriendlyName from Functiondiscoverykeys_devpkey.h.
// We keep this local to avoid SDK header macro conflicts across toolchains.
constexpr PROPERTYKEY kDeviceFriendlyNameKey{
    {0xa45c254e, 0xdf1c, 0x4efd, {0x80, 0x20, 0x67, 0xd1, 0x46, 0xa8, 0x50, 0xe0}},
    14};

std::string WideToUtf8(const wchar_t* wide) {
  if (wide == nullptr || *wide == L'\0') {
    return {};
  }

  const int needed = WideCharToMultiByte(CP_UTF8, 0, wide, -1, nullptr, 0, nullptr, nullptr);
  if (needed <= 1) {
    return {};
  }

  std::string utf8(static_cast<std::size_t>(needed - 1), '\0');
  WideCharToMultiByte(CP_UTF8, 0, wide, -1, utf8.data(), needed, nullptr, nullptr);
  return utf8;
}

std::vector<AudioDeviceInfo> EnumerateWasapiDevices(EDataFlow flow) {
  std::vector<AudioDeviceInfo> devices;

  const HRESULT init_hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
  const bool should_uninit = SUCCEEDED(init_hr);
  if (FAILED(init_hr) && init_hr != RPC_E_CHANGED_MODE) {
    return devices;
  }

  IMMDeviceEnumerator* enumerator = nullptr;
  HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                                __uuidof(IMMDeviceEnumerator),
                                reinterpret_cast<void**>(&enumerator));
  if (FAILED(hr) || enumerator == nullptr) {
    if (should_uninit) {
      CoUninitialize();
    }
    return devices;
  }

  std::string default_device_id;
  IMMDevice* default_device = nullptr;
  hr = enumerator->GetDefaultAudioEndpoint(flow, eConsole, &default_device);
  if (SUCCEEDED(hr) && default_device != nullptr) {
    LPWSTR default_wide_id = nullptr;
    if (SUCCEEDED(default_device->GetId(&default_wide_id)) &&
        default_wide_id != nullptr) {
      default_device_id = WideToUtf8(default_wide_id);
      CoTaskMemFree(default_wide_id);
    }
    default_device->Release();
  }

  IMMDeviceCollection* collection = nullptr;
  hr = enumerator->EnumAudioEndpoints(flow, DEVICE_STATE_ACTIVE, &collection);
  if (SUCCEEDED(hr) && collection != nullptr) {
    UINT count = 0;
    if (SUCCEEDED(collection->GetCount(&count))) {
      devices.reserve(static_cast<std::size_t>(count));
      for (UINT i = 0; i < count; ++i) {
        IMMDevice* device = nullptr;
        if (FAILED(collection->Item(i, &device)) || device == nullptr) {
          continue;
        }

        AudioDeviceInfo info;
        LPWSTR device_wide_id = nullptr;
        if (SUCCEEDED(device->GetId(&device_wide_id)) && device_wide_id != nullptr) {
          info.id = WideToUtf8(device_wide_id);
          CoTaskMemFree(device_wide_id);
        }

        IPropertyStore* props = nullptr;
        if (SUCCEEDED(device->OpenPropertyStore(STGM_READ, &props)) && props != nullptr) {
          PROPVARIANT value;
          PropVariantInit(&value);
          if (SUCCEEDED(props->GetValue(kDeviceFriendlyNameKey, &value)) &&
              value.vt == VT_LPWSTR) {
            info.name = WideToUtf8(value.pwszVal);
          }
          PropVariantClear(&value);
          props->Release();
        }

        if (info.name.empty()) {
          info.name = info.id;
        }
        info.is_default = (!default_device_id.empty() && info.id == default_device_id);
        if (!info.id.empty()) {
          devices.push_back(std::move(info));
        }

        device->Release();
      }
    }
    collection->Release();
  }

  enumerator->Release();
  if (should_uninit) {
    CoUninitialize();
  }
  return devices;
}

}  // namespace

std::vector<AudioDeviceInfo> EnumerateWasapiInputDevices() {
  return EnumerateWasapiDevices(eCapture);
}

std::vector<AudioDeviceInfo> EnumerateWasapiOutputDevices() {
  return EnumerateWasapiDevices(eRender);
}

}  // namespace tempolink::audio::platform::windows::detail
