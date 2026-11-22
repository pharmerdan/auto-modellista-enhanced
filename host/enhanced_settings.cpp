#include "enhanced_settings.hpp"

#include "ps2/os.hpp"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <filesystem>
#include <sstream>

namespace enhanced {
namespace {

Settings g_settings;

std::string trim(std::string text) {
    const auto space = [](unsigned char c) { return std::isspace(c) != 0; };
    text.erase(text.begin(), std::find_if_not(text.begin(), text.end(), space));
    text.erase(std::find_if_not(text.rbegin(), text.rend(), space).base(), text.end());
    return text;
}

bool boolean(const std::string &value, bool fallback) {
    if (value == "1" || value == "on" || value == "true" || value == "yes") return true;
    if (value == "0" || value == "off" || value == "false" || value == "no") return false;
    return fallback;
}

bool valid_aspect(const std::string &value) {
    return value == "auto" || value == "4:3" || value == "16:9" || value == "16:10" || value == "21:9" || value == "32:9";
}

float number(const std::string &value, float fallback, float low, float high) {
    char *end = nullptr;
    const float parsed = std::strtof(value.c_str(), &end);
    return end && end != value.c_str() ? std::clamp(parsed, low, high) : fallback;
}

}  // namespace

Settings defaults() { return {}; }

Settings parse(const std::string &text) {
    Settings out = defaults();
    std::istringstream input(text);
    std::string line;
    while (std::getline(input, line)) {
        const std::size_t comment = line.find_first_of("#;");
        if (comment != std::string::npos) line.resize(comment);
        const std::size_t equal = line.find('=');
        if (equal == std::string::npos) continue;
        std::string key = trim(line.substr(0, equal)), value = trim(line.substr(equal + 1));
        std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        if (key == "aspect" && valid_aspect(value)) out.aspect = value;
        else if (key == "scale") out.scale = std::clamp(std::atoi(value.c_str()), 1, 4);
        else if (key == "picture_look") out.look = std::clamp(std::atoi(value.c_str()), 0, 5);
        else if (key == "sharp_2d") out.sharp_2d = std::clamp(std::atoi(value.c_str()), 0, 2);
        else if (key == "image_filter") out.image_filter = value == "cas" ? 1 : value == "original" ? 0 : std::clamp(std::atoi(value.c_str()), 0, 1);
        else if (key == "sharpening") out.sharpening = number(value, out.sharpening, 0.0f, 1.0f);
        else if (key == "fullscreen") out.fullscreen = boolean(value, out.fullscreen);
        else if (key == "vsync") out.vsync = boolean(value, out.vsync);
        else if (key == "performance_overlay") out.performance_overlay = boolean(value, out.performance_overlay);
        else if (key == "crt_profile") out.crt_profile = std::clamp(std::atoi(value.c_str()), 0, 3);
        else if (key == "crt_scanline") out.crt_scanline = number(value, out.crt_scanline, 0.0f, 1.0f);
        else if (key == "crt_mask") out.crt_mask = number(value, out.crt_mask, 0.0f, 1.0f);
        else if (key == "crt_curvature") out.crt_curvature = number(value, out.crt_curvature, 0.0f, 1.0f);
        else if (key == "crt_brightness") out.crt_brightness = number(value, out.crt_brightness, 0.5f, 2.0f);
        else if (key == "crt_sharpness") out.crt_sharpness = number(value, out.crt_sharpness, 0.0f, 1.0f);
        else if (key == "hud_edge_anchored") out.hud_edge_anchored = boolean(value, out.hud_edge_anchored);
        else if (key == "hud_margin") out.hud_margin = std::clamp(std::atoi(value.c_str()), 0, 64);
    }
    return out;
}

std::string serialize(const Settings &s) {
    return "# Auto Modellista Enhanced graphics settings\n"
           "# Precedence: command line > PS2_* environment > this file > defaults.\n"
           "aspect = " + s.aspect + "\n" +
           "scale = " + std::to_string(s.scale) + "\n" +
           "picture_look = " + std::to_string(s.look) + "\n" +
           "sharp_2d = " + std::to_string(s.sharp_2d) + "\n" +
           "image_filter = " + std::string(s.image_filter == 1 ? "cas" : "original") + "\n" +
           "sharpening = " + std::to_string(s.sharpening) + "\n" +
           "fullscreen = " + std::string(s.fullscreen ? "on" : "off") + "\n" +
           "vsync = " + std::string(s.vsync ? "on" : "off") + "\n" +
           "performance_overlay = " + std::string(s.performance_overlay ? "on" : "off") + "\n" +
           "crt_profile = " + std::to_string(s.crt_profile) + "\n" +
           "crt_scanline = " + std::to_string(s.crt_scanline) + "\n" +
           "crt_mask = " + std::to_string(s.crt_mask) + "\n" +
           "crt_curvature = " + std::to_string(s.crt_curvature) + "\n" +
           "crt_brightness = " + std::to_string(s.crt_brightness) + "\n" +
           "crt_sharpness = " + std::to_string(s.crt_sharpness) + "\n" +
           "hud_edge_anchored = " + std::string(s.hud_edge_anchored ? "on" : "off") + "\n" +
           "hud_margin = " + std::to_string(s.hud_margin) + "\n";
}

std::string settings_path() {
    if (const std::string forced = ps2::os::env("PS2_ENHANCED_SETTINGS"); !forced.empty()) return forced;
    std::filesystem::path base;
#ifdef _WIN32
    if (const std::string appdata = ps2::os::env("APPDATA"); !appdata.empty()) base = ps2::os::path(appdata) / "ps2-native-recomp";
#elif defined(__APPLE__)
    if (const std::string home = ps2::os::env("HOME"); !home.empty()) base = ps2::os::path(home) / "Library" / "Application Support" / "ps2-native-recomp";
#else
    if (const std::string xdg = ps2::os::env("XDG_CONFIG_HOME"); !xdg.empty()) base = ps2::os::path(xdg) / "ps2-native-recomp";
    else if (const std::string home = ps2::os::env("HOME"); !home.empty()) base = ps2::os::path(home) / ".config" / "ps2-native-recomp";
#endif
    if (base.empty()) base = std::filesystem::temp_directory_path() / "ps2-native-recomp";
    return ps2::os::utf8(base / "automodellista-enhanced.ini");
}

bool load() {
    const std::string path = settings_path();
    std::string text;
    if (std::FILE *file = ps2::os::open(path, "rb")) {
        char buffer[2048];
        std::size_t n = 0;
        while ((n = std::fread(buffer, 1, sizeof buffer, file)) != 0) text.append(buffer, n);
        std::fclose(file);
        g_settings = parse(text);
        std::fprintf(stderr, "[enhanced] settings loaded from %s\n", path.c_str());
        return true;
    }
    g_settings = defaults();
    return false;
}

bool save() {
    const std::string path = settings_path();
    std::error_code error;
    std::filesystem::create_directories(ps2::os::path(path).parent_path(), error);
    if (std::FILE *file = ps2::os::open(path, "wb")) {
        const std::string text = serialize(g_settings);
        const bool ok = std::fwrite(text.data(), 1, text.size(), file) == text.size() && std::fclose(file) == 0;
        if (ok) return true;
    }
    std::fprintf(stderr, "[enhanced] cannot write settings to %s\n", path.c_str());
    return false;
}

void restore_defaults() { g_settings = defaults(); }
Settings &settings() { return g_settings; }

}  // namespace enhanced
