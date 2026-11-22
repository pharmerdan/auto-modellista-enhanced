#include "gs/gpu/image.hpp"

#include <cstdio>
#include <string>

int main() {
    using namespace ps2::gs::gpu;
    int failed = 0;
    const auto check = [&](bool value, const char *name) {
        std::printf("%s %s\n", value ? "PASS" : "FAIL", name);
        failed += !value;
    };
    check(!image_enhancement_enabled(ImageSettings{}), "original image path is the default");
    check(std::string(image_filter_name(ImageFilter::Original)) == "Original", "original filter name");
    ImageSettings cas{ImageFilter::Cas, 0.6f};
    check(image_enhancement_enabled(cas) && std::string(image_filter_name(cas.filter)) == "CAS", "CAS selects enhancement path");
    check(sanitize({ImageFilter::Cas, -2.0f}).sharpening == 0.0f, "sharpening lower limit");
    check(sanitize({ImageFilter::Cas, 4.0f}).sharpening == 1.0f, "sharpening upper limit");
    const ImageSettings invalid = sanitize({static_cast<ImageFilter>(99), 0.4f});
    check(invalid.filter == ImageFilter::Cas, "image filter enum is clamped");
    return failed ? 1 : 0;
}
