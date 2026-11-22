#include "gs/gpu/crt.hpp"

#include <cstdio>
#include <string>

int main() {
    int failed = 0;
    const auto check = [&](bool value, const char *name) {
        std::printf("%s %s\n", value ? "PASS" : "FAIL", name);
        failed += !value;
    };
    using namespace ps2::gs::gpu;
    check(std::string(crt_profile_name(CrtProfile::Off)) == "Off", "disabled profile is explicit");
    check(!crt_enabled(CrtSettings{}), "disabled presentation path stays selected by default");
    CrtSettings fast{}; fast.profile = CrtProfile::LottesFast;
    check(crt_enabled(fast), "shader profile selects the CRT presentation path");
    check(std::string(crt_profile_name(CrtProfile::LottesFast)) == "CRT-Lottes Fast", "fast profile name");
    CrtSettings bad{static_cast<CrtProfile>(99), -1.0f, 2.0f, 4.0f, 9.0f, -2.0f};
    const CrtSettings fixed = sanitize(bad);
    check(fixed.profile == CrtProfile::Hyllian, "profile selection clamps");
    check(fixed.scanline == 0.0f && fixed.mask == 1.0f && fixed.curvature == 1.0f, "effect parameters clamp");
    check(fixed.brightness == 2.0f && fixed.sharpness == 0.0f, "brightness and sharpness clamp");
    return failed ? 1 : 0;
}
