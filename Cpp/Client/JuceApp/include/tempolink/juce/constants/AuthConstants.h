#pragma once

#include <cstdint>

namespace tempolink::juceapp::constants {

inline constexpr char kOAuthCallbackHost[] = "127.0.0.1";
inline constexpr std::uint16_t kOAuthCallbackPort = 35999;
inline constexpr char kOAuthCallbackPath[] = "/oauth/callback";

}  // namespace tempolink::juceapp::constants

