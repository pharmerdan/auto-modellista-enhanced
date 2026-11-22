#include "../host/enhanced_settings.hpp"
#include "ps2/os.hpp"

#include <cstdio>
#include <filesystem>
#include <string>

int main() {
    int failed = 0;
    const auto check = [&](bool value, const char *name) {
        std::printf("%s %s\n", value ? "PASS" : "FAIL", name);
        failed += !value;
    };
    enhanced::Settings s = enhanced::parse("aspect = 21:9\nscale=4\npicture_look=2\nsharp_2d=0\nimage_filter=cas\nsharpening=.7\nfullscreen=yes\nvsync=off\nperformance_overlay=on\n"
                                            "crt_profile=2\ncrt_scanline=.8\ncrt_mask=.4\ncrt_curvature=.2\ncrt_brightness=1.25\ncrt_sharpness=.6\n"
                                            "hud_edge_anchored=on\nhud_margin=28\n");
    check(s.aspect == "21:9" && s.scale == 4, "display values parse");
    check(s.look == 2 && s.sharp_2d == 0, "graphics values parse");
    check(s.image_filter == 1 && s.sharpening == .7f, "image enhancement values parse");
    check(s.fullscreen && !s.vsync && s.performance_overlay, "boolean values parse");
    check(s.crt_profile == 2 && s.crt_scanline == .8f && s.crt_mask == .4f && s.crt_brightness == 1.25f, "CRT values parse");
    check(s.hud_edge_anchored && s.hud_margin == 28, "HUD layout values parse");
    const enhanced::Settings roundtrip = enhanced::parse(enhanced::serialize(s));
    check(roundtrip.aspect == s.aspect && roundtrip.scale == s.scale && roundtrip.look == s.look && roundtrip.sharp_2d == s.sharp_2d &&
              roundtrip.fullscreen == s.fullscreen && roundtrip.vsync == s.vsync && roundtrip.performance_overlay == s.performance_overlay &&
              roundtrip.image_filter == s.image_filter && roundtrip.sharpening == s.sharpening &&
              roundtrip.crt_profile == s.crt_profile && roundtrip.crt_scanline == s.crt_scanline && roundtrip.crt_mask == s.crt_mask &&
              roundtrip.hud_edge_anchored == s.hud_edge_anchored && roundtrip.hud_margin == s.hud_margin,
          "settings serialize and parse round trip");
    const enhanced::Settings bad = enhanced::parse("aspect=nonsense\nscale=99\npicture_look=-3\nsharp_2d=8\nimage_filter=9\nsharpening=4\ncrt_profile=99\ncrt_scanline=8\nhud_margin=-2\n");
    check(bad.aspect == "16:9" && bad.scale == 4 && bad.look == 0 && bad.sharp_2d == 2, "invalid values are rejected or clamped");
    check(bad.crt_profile == 3 && bad.crt_scanline == 1.0f && bad.hud_margin == 0, "new values are clamped");
    check(bad.image_filter == 1 && bad.sharpening == 1.0f, "image enhancement values are clamped");
    const std::filesystem::path path = std::filesystem::temp_directory_path() / "ps2-enhanced-settings-test.ini";
    ps2::os::set_env("PS2_ENHANCED_SETTINGS", ps2::os::utf8(path));
    enhanced::settings() = s;
    check(enhanced::save(), "settings file saves");
    enhanced::restore_defaults();
    check(enhanced::load() && enhanced::settings().aspect == "21:9" && enhanced::settings().scale == 4 && !enhanced::settings().vsync,
          "settings persist between loads");
    std::error_code error;
    std::filesystem::remove(path, error);
    return failed ? 1 : 0;
}
