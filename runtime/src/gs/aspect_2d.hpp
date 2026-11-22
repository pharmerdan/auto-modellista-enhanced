#pragma once

#include "gs.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace ps2::gs::aspect2d {

// Auto Modellista draws its HUD with the same GIF path as the world. Use the
// game's overlay depth layer first, then distinguish projected geometry before
// looking at the depth-write mask: car shadows test depth without writing it.
struct Signature {
    std::uint32_t prim = 0;
    std::uint64_t attr = 0;
    const Context *context = nullptr;
    const Vertex *vertices = nullptr;
    int count = 0;
};

inline bool depth_masked(const Signature &s) { return s.context && ((s.context->zbuf >> 32) & 1u); }
inline bool depth_tested(const Signature &s) { return s.context && ((s.context->test >> 16) & 1u); }
inline bool textured(const Signature &s) { return (s.attr & 0x10u) != 0; }
inline bool blended(const Signature &s) { return (s.attr & 0x40u) != 0; }

inline bool flat_zq(const Signature &s) {
    if (!s.vertices || s.count < 1) return false;
    const Vertex &a = s.vertices[0];
    for (int i = 1; i < s.count; ++i) {
        const Vertex &b = s.vertices[i];
        if (a.z != b.z || !std::isfinite(a.q) || !std::isfinite(b.q) || a.q != b.q) return false;
    }
    return true;
}

// The later frontend uses textured triangle meshes rather than GS sprites.
// They test and write depth, which is why the older masked-depth rule did not
// recognize them. Every observed frontend mesh triangle has constant Z/Q;
// the projected car preview in the same frame varies them. Do not require an
// axis-aligned edge: that recognized only selected halves of curved panels.
// Race diagnostics also found a few flat, nearly degenerate projected
// triangles, so require at least one console pixel of extent on both axes.
inline bool frontend_triangle(const Signature &s) {
    if (s.prim < 3 || s.prim > 5 || s.count != 3 || !s.vertices || !depth_tested(s) || depth_masked(s) ||
        !textured(s) || !blended(s) || !flat_zq(s)) return false;
    const Vertex &a = s.vertices[0], &b = s.vertices[1], &c = s.vertices[2];
    const std::uint32_t xmin = std::min({a.x, b.x, c.x}), xmax = std::max({a.x, b.x, c.x});
    const std::uint32_t ymin = std::min({a.y, b.y, c.y}), ymax = std::max({a.y, b.y, c.y});
    return xmax - xmin >= 16u && ymax - ymin >= 16u;
}

inline bool projected_geometry(const Signature &s) {
    if (s.prim < 3 || s.prim > 5 || s.count < 3 || !s.vertices || !depth_tested(s)) return false;
    const Vertex &a = s.vertices[0];
    for (int i = 1; i < s.count; ++i) {
        const Vertex &b = s.vertices[i];
        if (a.z != b.z || (std::isfinite(a.q) && std::isfinite(b.q) && std::fabs(a.q - b.q) > 0.000001f)) return true;
    }
    return false;
}

// A useful diagnostic subset of projected geometry.  It matches the render
// state normally used by shadows without making texture or blending alone a
// reason to classify a draw as a shadow.
inline bool shadow_like(const Signature &s) {
    return depth_masked(s) && textured(s) && blended(s) && projected_geometry(s);
}

// Auto Modellista places screen-space overlays near the top of the 24-bit Z
// range. Their GIF batches can still contain changing Z/Q values, so those
// values alone are not proof that a triangle came from the 3D projection.
inline bool interface_depth_layer(const Signature &s) {
    if (!s.vertices || s.count < 1) return false;
    // High-Z sprites are the game's other established UI path. Triangles must
    // also mask depth writes so high-Z world geometry cannot enter this path.
    if (s.prim != 6 && !depth_masked(s)) return false;
    for (int i = 0; i < s.count; ++i)
        if (s.vertices[i].z < 0x00700000u) return false;
    return true;
}

inline std::uint32_t frame_width(const Context &c) {
    const std::uint32_t fbw = static_cast<std::uint32_t>(c.frame >> 16) & 0x3Fu;
    return std::max(64u, fbw * 64u);
}

inline bool full_surface_sprite(const Signature &s) {
    if (s.prim != 6 || s.count < 2 || !s.vertices || !s.context) return false;
    const int ofx = static_cast<int>(s.context->xyoffset & 0xFFFFu);
    const int ofy = static_cast<int>((s.context->xyoffset >> 32) & 0xFFFFu);
    const auto px = [ofx](const Vertex &v) { return (static_cast<int>(v.x) - ofx) / 16; };
    const auto py = [ofy](const Vertex &v) { return (static_cast<int>(v.y) - ofy) / 16; };
    const int x0 = std::min(px(s.vertices[0]), px(s.vertices[1]));
    const int x1 = std::max(px(s.vertices[0]), px(s.vertices[1]));
    const int y0 = std::min(py(s.vertices[0]), py(s.vertices[1]));
    const int y1 = std::max(py(s.vertices[0]), py(s.vertices[1]));
    const std::uint32_t sy1 = static_cast<std::uint32_t>(s.context->scissor >> 48) & 0x7FFu;
    return x0 <= 0 && x1 >= static_cast<int>(frame_width(*s.context)) && y0 <= 0 && y1 >= static_cast<int>(sy1 + 1);
}

enum class DrawClass : std::uint8_t { Original, Interface, ProjectedGeometry, FullSurface };

inline DrawClass classify(const Signature &s) {
    if (full_surface_sprite(s)) return DrawClass::FullSurface;
    // Check the game's explicit overlay depth layer before perspective values:
    // UI batches may carry varying Z/Q, while shadows live below this layer.
    if (interface_depth_layer(s)) return DrawClass::Interface;
    // The later frontend's lower-Z triangle path writes depth. Its constant
    // projection coordinates distinguish it from the projected car preview.
    if (frontend_triangle(s)) return DrawClass::Interface;
    // This check must still precede depth_masked(). Shadows commonly use ZMSK=1.
    if (projected_geometry(s)) return DrawClass::ProjectedGeometry;
    if (depth_masked(s)) return DrawClass::Interface;
    return DrawClass::Original;
}

inline bool correct(const Signature &s) { return classify(s) == DrawClass::Interface; }

inline std::uint32_t corrected_x(std::uint32_t raw, std::uint32_t ofx, std::uint32_t width, float factor) {
    const double x = (static_cast<double>(raw) - static_cast<double>(ofx)) / 16.0;
    const double centre = static_cast<double>(width) * 0.5;
    const double moved = centre + (x - centre) * static_cast<double>(factor);
    const double result = static_cast<double>(ofx) + moved * 16.0;
    return static_cast<std::uint32_t>(std::clamp(std::llround(result), 0ll, 0xFFFFFFFFll));
}

inline std::uint64_t corrected_scissor(std::uint64_t value, std::uint32_t width, float factor) {
    const int x0 = static_cast<int>(value & 0x7FFu);
    const int x1 = static_cast<int>((value >> 16) & 0x7FFu);
    // A full-frame scissor is shared by clears, world geometry and UI in this
    // game. Keep it stable; corrected UI vertices are already inside it.
    if (x0 <= 0 && x1 >= static_cast<int>(width) - 1) return value;
    const double centre = static_cast<double>(width) * 0.5;
    // GS scissors are inclusive. Round the half-open interval [x0, x1 + 1)
    // outwards so conversion never drops an edge pixel.
    const int moved0 = static_cast<int>(std::floor(centre + (static_cast<double>(x0) - centre) * factor));
    const int moved1 = static_cast<int>(std::ceil(centre + (static_cast<double>(x1 + 1) - centre) * factor)) - 1;
    const std::uint64_t lo = static_cast<std::uint64_t>(std::clamp(moved0, 0, 2047));
    const std::uint64_t hi = static_cast<std::uint64_t>(std::clamp(moved1, 0, 2047));
    return (value & ~0x07FF07FFull) | lo | (hi << 16);
}

}  // namespace ps2::gs::aspect2d
