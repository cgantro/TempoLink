#include "tempolink/juce/network/http/common/JsonParse.h"

#include <cctype>
#include <cstdint>
#include <exception>

namespace tempolink::juceapp::jsonparse {

Json Parse(const juce::String& text) {
  return Json::parse(text.toStdString(), nullptr, false);
}

std::string String(const Json& obj, const char* key, const std::string& fallback) {
  const auto it = obj.find(key);
  if (it == obj.end() || it->is_null()) {
    return fallback;
  }
  if (it->is_string()) {
    return it->get<std::string>();
  }
  if (it->is_boolean()) {
    return it->get<bool>() ? "true" : "false";
  }
  if (it->is_number_integer()) {
    return std::to_string(it->get<long long>());
  }
  if (it->is_number_unsigned()) {
    return std::to_string(it->get<unsigned long long>());
  }
  if (it->is_number_float()) {
    return std::to_string(it->get<double>());
  }
  return fallback;
}

int Int(const Json& obj, const char* key, int fallback) {
  const auto it = obj.find(key);
  if (it == obj.end() || it->is_null()) {
    return fallback;
  }
  try {
    if (it->is_number_integer()) {
      return it->get<int>();
    }
    if (it->is_number_unsigned()) {
      return static_cast<int>(it->get<unsigned int>());
    }
    if (it->is_number_float()) {
      return static_cast<int>(it->get<double>());
    }
    if (it->is_string()) {
      return std::stoi(it->get<std::string>());
    }
  } catch (const std::exception&) {
    return fallback;
  }
  return fallback;
}

long Long(const Json& obj, const char* key, long fallback) {
  const auto it = obj.find(key);
  if (it == obj.end() || it->is_null()) {
    return fallback;
  }
  try {
    if (it->is_number_integer()) {
      return it->get<long>();
    }
    if (it->is_number_unsigned()) {
      return static_cast<long>(it->get<unsigned long>());
    }
    if (it->is_number_float()) {
      return static_cast<long>(it->get<double>());
    }
    if (it->is_string()) {
      return std::stol(it->get<std::string>());
    }
  } catch (const std::exception&) {
    return fallback;
  }
  return fallback;
}

std::int64_t Int64(const Json& obj, const char* key, std::int64_t fallback) {
  const auto it = obj.find(key);
  if (it == obj.end() || it->is_null()) {
    return fallback;
  }
  try {
    if (it->is_number_integer()) {
      return it->get<std::int64_t>();
    }
    if (it->is_number_unsigned()) {
      return static_cast<std::int64_t>(it->get<std::uint64_t>());
    }
    if (it->is_number_float()) {
      return static_cast<std::int64_t>(it->get<double>());
    }
    if (it->is_string()) {
      return std::stoll(it->get<std::string>());
    }
  } catch (const std::exception&) {
    return fallback;
  }
  return fallback;
}

bool Bool(const Json& obj, const char* key, bool fallback) {
  const auto it = obj.find(key);
  if (it == obj.end() || it->is_null()) {
    return fallback;
  }
  if (it->is_boolean()) {
    return it->get<bool>();
  }
  if (it->is_number()) {
    return it->get<double>() != 0.0;
  }
  if (it->is_string()) {
    auto s = it->get<std::string>();
    for (auto& ch : s) {
      ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    if (s == "true" || s == "1" || s == "yes" || s == "on") {
      return true;
    }
    if (s == "false" || s == "0" || s == "no" || s == "off") {
      return false;
    }
  }
  return fallback;
}

}  // namespace tempolink::juceapp::jsonparse
