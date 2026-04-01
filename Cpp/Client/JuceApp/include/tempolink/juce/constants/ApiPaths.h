#pragma once

namespace tempolink::juceapp::constants {

inline constexpr char kUsersPath[] = "/api/users";
inline constexpr char kNewsPath[] = "/api/notices";
inline constexpr char kManualPath[] = "/api/manual";
inline constexpr char kFaqPath[] = "/api/faq";
inline constexpr char kAuthProvidersPath[] = "/api/auth/providers";
inline constexpr char kAuthSocialStartPrefix[] = "/api/auth/social/";
inline constexpr char kAuthTicketPrefix[] = "/api/auth/social/ticket/";
inline constexpr char kProfileMePath[] = "/api/profile/me";

inline constexpr char kManualFallbackDocPath[] =
    "Docs/Architecture/README.md";
inline constexpr char kFaqFallbackDocPath[] =
    "Docs/Architecture/LowLatencyPlan.md";

}  // namespace tempolink::juceapp::constants
