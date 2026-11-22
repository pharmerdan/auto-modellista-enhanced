#pragma once

#include <string>

// Persistent settings for the enhanced in-game frontend. This module deliberately
// has no SDL or platform graphics dependencies; platform-specific path selection is
// confined to settings_path().
namespace enhanced {

struct Settings {
    std::string aspect = "16:9";
    int scale = 2;
    int look = 5;
    int sharp_2d = 1;
    int image_filter = 0; // 0 original, 1 Contrast Adaptive Sharpening
    float sharpening = 0.35f;
    bool fullscreen = false;
    bool vsync = true;
    bool performance_overlay = false;
    int crt_profile = 0; // 0 off, 1 Lottes Fast, 2 Lottes, 3 Hyllian
    float crt_scanline = 0.65f;
    float crt_mask = 0.35f;
    float crt_curvature = 0.10f;
    float crt_brightness = 1.12f;
    float crt_sharpness = 0.35f;
    bool hud_edge_anchored = false;
    int hud_margin = 16;
};

Settings defaults();
Settings parse(const std::string &text);
std::string serialize(const Settings &settings);
std::string settings_path();
bool load();
bool save();
void restore_defaults();
Settings &settings();

}  // namespace enhanced
