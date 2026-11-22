#pragma once

#include <algorithm>
#include <cstdint>

namespace ps2::gs::gpu {

enum class CrtProfile : std::uint32_t { Off, LottesFast, Lottes, Hyllian };

struct CrtSettings {
    CrtProfile profile = CrtProfile::Off;
    float scanline = 0.65f;
    float mask = 0.35f;
    float curvature = 0.10f;
    float brightness = 1.12f;
    float sharpness = 0.35f;
};

inline CrtSettings sanitize(CrtSettings value) {
    value.profile = static_cast<CrtProfile>(std::clamp(static_cast<std::uint32_t>(value.profile), 0u, 3u));
    value.scanline = std::clamp(value.scanline, 0.0f, 1.0f);
    value.mask = std::clamp(value.mask, 0.0f, 1.0f);
    value.curvature = std::clamp(value.curvature, 0.0f, 1.0f);
    value.brightness = std::clamp(value.brightness, 0.5f, 2.0f);
    value.sharpness = std::clamp(value.sharpness, 0.0f, 1.0f);
    return value;
}

inline const char *crt_profile_name(CrtProfile profile) {
    switch (profile) {
    case CrtProfile::Off: return "Off";
    case CrtProfile::LottesFast: return "CRT-Lottes Fast";
    case CrtProfile::Lottes: return "CRT-Lottes";
    case CrtProfile::Hyllian: return "CRT-Hyllian";
    }
    return "Off";
}

inline bool crt_enabled(const CrtSettings &settings) { return sanitize(settings).profile != CrtProfile::Off; }

} // namespace ps2::gs::gpu
