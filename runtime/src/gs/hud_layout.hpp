#pragma once

#include "aspect_2d.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace ps2::gs::hud {

enum class Group : std::uint8_t { None, LeftRace, RightRace, Speedometer };

inline std::uint32_t texture_base_pointer(const Context &c) {
    // TEX0.TBP0 is compared in the same raw GS units printed by the draw log.
    // The captured PAL values below are field values, not byte addresses.
    return static_cast<std::uint32_t>(c.tex0 & 0x3fffu);
}

inline bool known_race_hud_page(std::uint32_t tbp) {
    // PAL SLES-51191 uses the first set after a normal boot and the second in
    // captured/restored GS states. These are the observed race HUD atlas,
    // lap-counter atlas and course-map pages, not a generic texture rule.
    switch (tbp) {
    case 0x2b00: case 0x2c00: case 0x2e00:
    case 0x37a0: case 0x38a0: case 0x3aa0: return true;
    default: return false;
    }
}

inline bool known_speedometer_page(std::uint32_t tbp) {
    // Dedicated speedometer atlas. 2d00 is the normal-boot page and 39a0 is
    // the corresponding page in the captured/restored PAL GS state.
    return tbp == 0x2d00 || tbp == 0x39a0;
}

inline bool known_cluster_sprite_page(std::uint32_t tbp) {
    // The digital/gear sprites use the main HUD atlas, not the lap or map
    // pages which happen to share the rest of the sprite render state.
    return tbp == 0x2b00 || tbp == 0x37a0;
}

struct Bounds {
    int x0, y0, x1, y1;
};

inline Bounds bounds(const aspect2d::Signature &s) {
    const int ofx = static_cast<int>(s.context->xyoffset & 0xffffu);
    const int ofy = static_cast<int>((s.context->xyoffset >> 32) & 0xffffu);
    Bounds b{0x7fffffff, 0x7fffffff, -0x7fffffff, -0x7fffffff};
    for (int i = 0; i < s.count; ++i) {
        const int x = (static_cast<int>(s.vertices[i].x) - ofx) / 16;
        const int y = (static_cast<int>(s.vertices[i].y) - ofy) / 16;
        b.x0 = std::min(b.x0, x); b.x1 = std::max(b.x1, x);
        b.y0 = std::min(b.y0, y); b.y1 = std::max(b.y1, y);
    }
    return b;
}

// Positively identified from the PAL GS stream and a frame-4580 capture. The
// position/times occupy x=22..151,y=25..202; lap/map x=486..616,y=19..181;
// the complete speedometer cluster occupies x=369..612,y=247..426.
inline Group identify(const aspect2d::Signature &s) {
    if (!s.context || !s.vertices || s.count < 1 || !aspect2d::textured(s) || !aspect2d::blended(s)) return Group::None;
    const std::uint32_t tbp = texture_base_pointer(*s.context);
    const Bounds b = bounds(s);

    if (s.prim == 6 && s.count == 2 && aspect2d::interface_depth_layer(s) && known_race_hud_page(tbp)) {
        if (b.x0 >= 18 && b.x1 <= 155 && b.y0 >= 15 && b.y1 <= 210) return Group::LeftRace;
        if (b.x0 >= 475 && b.x1 <= 622 && b.y0 >= 10 && b.y1 <= 210) return Group::RightRace;
        // The five captured sprites immediately following the dial mesh are
        // the gear, three digital-speed digits and their associated label.
        if (known_cluster_sprite_page(tbp) && b.x0 >= 370 && b.x1 <= 485 && b.y0 >= 360 && b.y1 <= 425)
            return Group::Speedometer;
    }

    // PAL frame 4580 draws the dial, tachometer/needle and auxiliary gauge as
    // 43 triangles from the dedicated 39a0 atlas, bounded by 369..612 and
    // 247..426. Require that exact triangle path and render state; this is not
    // a generic sprite, depth or screen-corner heuristic.
    const std::uint64_t path = s.attr & (0x8u | 0x10u | 0x20u | 0x40u | 0x100u);
    if (s.prim == 4 && s.count == 3 && path == (0x8u | 0x10u | 0x20u | 0x40u) &&
        aspect2d::depth_masked(s) && aspect2d::depth_tested(s) && known_speedometer_page(tbp) &&
        b.x0 >= 365 && b.x1 <= 615 && b.y0 >= 240 && b.y1 <= 430)
        return Group::Speedometer;
    return Group::None;
}

inline int edge_shift(Group group, std::uint32_t width, float factor, int margin) {
    if (group == Group::None || factor >= 0.9999f) return 0;
    const int safe_inset = static_cast<int>(std::lround(static_cast<double>(width) * (1.0 - factor) * 0.5));
    const int amount = std::max(0, safe_inset - std::clamp(margin, 0, 64));
    return group == Group::LeftRace ? -amount : amount;
}

inline std::uint32_t shifted_x(std::uint32_t raw, int pixels) {
    return static_cast<std::uint32_t>(std::clamp<std::int64_t>(static_cast<std::int64_t>(raw) + static_cast<std::int64_t>(pixels) * 16, 0, 0xffffffffll));
}

inline std::uint64_t shifted_scissor(std::uint64_t value, std::uint32_t width, int pixels) {
    const int x0 = static_cast<int>(value & 0x7ffu);
    const int x1 = static_cast<int>((value >> 16) & 0x7ffu);
    if (!pixels || (x0 <= 0 && x1 >= static_cast<int>(width) - 1)) return value;
    const std::uint64_t lo = static_cast<std::uint64_t>(std::clamp(x0 + pixels, 0, 2047));
    const std::uint64_t hi = static_cast<std::uint64_t>(std::clamp(x1 + pixels, 0, 2047));
    return (value & ~0x07ff07ffull) | lo | (hi << 16);
}

} // namespace ps2::gs::hud
