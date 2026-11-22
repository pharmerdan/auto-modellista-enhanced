#pragma once

#include <algorithm>
#include <cstdint>

namespace ps2::gs::gpu {

enum class ImageFilter : std::uint32_t { Original, Cas };

struct ImageSettings {
    ImageFilter filter = ImageFilter::Original;
    float sharpening = 0.35f;
};

inline ImageSettings sanitize(ImageSettings value) {
    value.filter = static_cast<ImageFilter>(std::clamp(static_cast<std::uint32_t>(value.filter), 0u, 1u));
    value.sharpening = std::clamp(value.sharpening, 0.0f, 1.0f);
    return value;
}

inline const char *image_filter_name(ImageFilter filter) {
    switch (filter) {
    case ImageFilter::Original: return "Original";
    case ImageFilter::Cas: return "CAS";
    }
    return "Original";
}

inline bool image_enhancement_enabled(const ImageSettings &settings) {
    return sanitize(settings).filter != ImageFilter::Original;
}

} // namespace ps2::gs::gpu
