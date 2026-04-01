#pragma once

#include <string>
#include <cstdint>

#include <juce_core/juce_core.h>
#include <nlohmann/json.hpp>

namespace tempolink::juceapp::jsonparse {

using Json = nlohmann::json;

Json Parse(const juce::String& text);
std::string String(const Json& obj, const char* key, const std::string& fallback = "");
int Int(const Json& obj, const char* key, int fallback = 0);
long Long(const Json& obj, const char* key, long fallback = 0);
std::int64_t Int64(const Json& obj, const char* key, std::int64_t fallback = 0);
bool Bool(const Json& obj, const char* key, bool fallback = false);

}  // namespace tempolink::juceapp::jsonparse
