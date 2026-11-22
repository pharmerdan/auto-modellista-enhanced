#include "ps2/system.hpp"
#include "gs/aspect_2d.hpp"
#include "gs/hud_layout.hpp"

#include <cmath>
#include <cstdio>
#include <string>

namespace {

int failures = 0;

#define CHECK(value) do { \
    if (!(value)) { \
        std::fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__, #value); \
        ++failures; \
    } \
} while (false)

using ps2::gs::Context;
using ps2::gs::Vertex;
using ps2::gs::aspect2d::DrawClass;
using ps2::gs::aspect2d::Signature;

Context context() {
    Context c{};
    c.frame = 10ull << 16; // 640 pixels
    c.scissor = 639ull << 16 | 447ull << 48;
    c.test = 1ull << 16;
    return c;
}

void classification() {
    Context c = context();
    c.zbuf = 1ull << 32;
    Vertex hud[3]{};
    hud[0].z = hud[1].z = hud[2].z = 0x00ffff28u;
    hud[0].q = hud[1].q = hud[2].q = 1.0f;
    const Signature masked_hud{4, 0x50u, &c, hud, 3};
    CHECK(ps2::gs::aspect2d::classify(masked_hud) == DrawClass::Interface);
    CHECK(ps2::gs::aspect2d::correct(masked_hud));

    c.zbuf = 0;
    Vertex world[3]{};
    world[0].z = 1; world[1].z = 2; world[2].z = 3;
    world[0].q = 0.1f; world[1].q = 0.2f; world[2].q = 0.3f;
    const Signature projected_world{4, 0x20u, &c, world, 3};
    CHECK(ps2::gs::aspect2d::projected_geometry(projected_world));
    CHECK(ps2::gs::aspect2d::classify(projected_world) == DrawClass::ProjectedGeometry);
    CHECK(!ps2::gs::aspect2d::correct(projected_world));

    // Regression: Auto Modellista UI triangles can carry perspective-varying
    // values. Their high Z layer must win over the generic 3D heuristic.
    c.zbuf = 1ull << 32;
    Vertex perspective_ui[3]{};
    perspective_ui[0].z = 0x00ffff00u;
    perspective_ui[1].z = 0x00ffff10u;
    perspective_ui[2].z = 0x00ffff20u;
    perspective_ui[0].q = 0.7f;
    perspective_ui[1].q = 0.8f;
    perspective_ui[2].q = 0.9f;
    const Signature batched_ui{4, 0x50u, &c, perspective_ui, 3};
    CHECK(ps2::gs::aspect2d::projected_geometry(batched_ui));
    CHECK(ps2::gs::aspect2d::interface_depth_layer(batched_ui));
    CHECK(ps2::gs::aspect2d::classify(batched_ui) == DrawClass::Interface);
    CHECK(ps2::gs::aspect2d::correct(batched_ui));

    // The same coordinates with depth writes enabled are genuine projected
    // geometry, not UI merely because their Z happens to be high.
    c.zbuf = 0;
    CHECK(!ps2::gs::aspect2d::interface_depth_layer(batched_ui));
    CHECK(ps2::gs::aspect2d::classify(batched_ui) == DrawClass::ProjectedGeometry);
    CHECK(!ps2::gs::aspect2d::correct(batched_ui));

    // The 50/60 Hz selector uses sprites, but later frontend screens use
    // textured, blended, depth-writing triangle pairs for screen-space quads.
    Vertex menu[3]{};
    menu[0].x = 32 * 16;  menu[0].y = 40 * 16;
    menu[1].x = 200 * 16; menu[1].y = 40 * 16;
    menu[2].x = 200 * 16; menu[2].y = 96 * 16;
    menu[0].z = menu[1].z = menu[2].z = 0x0002c315u;
    menu[0].q = menu[1].q = menu[2].q = 0.0117786f;
    const Signature frontend_quad_half{4, 0x10u | 0x40u, &c, menu, 3};
    CHECK(ps2::gs::aspect2d::frontend_triangle(frontend_quad_half));
    CHECK(ps2::gs::aspect2d::classify(frontend_quad_half) == DrawClass::Interface);

    // A projected preview triangle is not captured even if one edge happens
    // to be horizontal and another vertical.
    menu[1].z += 1;
    menu[1].q += 0.001f;
    CHECK(!ps2::gs::aspect2d::frontend_triangle(frontend_quad_half));
    CHECK(ps2::gs::aspect2d::classify(frontend_quad_half) == DrawClass::ProjectedGeometry);

    // Curved and animated frontend panels are meshes, not just axis-aligned
    // quad halves. Constant projection coordinates still identify their 2D
    // path and ensure the whole mesh receives one transform.
    menu[1].z = menu[0].z;
    menu[1].q = menu[0].q;
    menu[1].y += 1;
    CHECK(ps2::gs::aspect2d::frontend_triangle(frontend_quad_half));
    CHECK(ps2::gs::aspect2d::classify(frontend_quad_half) == DrawClass::Interface);

    // Race captures contain occasional flat, nearly degenerate projected
    // triangles. They must not enter the frontend correction path.
    menu[0].x = 100 * 16; menu[0].y = 100 * 16;
    menu[1].x = 112 * 16; menu[1].y = 100 * 16 + 1;
    menu[2].x = 106 * 16; menu[2].y = 100 * 16 + 2;
    CHECK(!ps2::gs::aspect2d::frontend_triangle(frontend_quad_half));
    CHECK(ps2::gs::aspect2d::classify(frontend_quad_half) == DrawClass::Original);

    // Almost-flat projected geometry is not constant in projection space.
    menu[2].y = 104 * 16;
    menu[2].q = menu[0].q + 0.0000001f;
    CHECK(!ps2::gs::aspect2d::frontend_triangle(frontend_quad_half));
    CHECK(ps2::gs::aspect2d::classify(frontend_quad_half) == DrawClass::Original);
    CHECK(!ps2::gs::aspect2d::correct(frontend_quad_half));
}

void shadow() {
    Context c = context();
    c.zbuf = 1ull << 32;
    Vertex vertices[3]{};
    vertices[0].z = 0x1000; vertices[1].z = 0x1200; vertices[2].z = 0x1400;
    vertices[0].q = 0.7f; vertices[1].q = 0.8f; vertices[2].q = 0.9f;
    const Signature candidate{4, 0x10u | 0x40u, &c, vertices, 3};
    CHECK(ps2::gs::aspect2d::depth_masked(candidate));
    CHECK(ps2::gs::aspect2d::shadow_like(candidate));
    CHECK(ps2::gs::aspect2d::classify(candidate) == DrawClass::ProjectedGeometry);
    CHECK(!ps2::gs::aspect2d::correct(candidate));

    // A shadow just below the interface threshold remains projected too.
    vertices[0].z = 0x006ffffdu;
    vertices[1].z = 0x006ffffeu;
    vertices[2].z = 0x006fffffu;
    CHECK(!ps2::gs::aspect2d::interface_depth_layer(candidate));
    CHECK(ps2::gs::aspect2d::shadow_like(candidate));
    CHECK(ps2::gs::aspect2d::classify(candidate) == DrawClass::ProjectedGeometry);
    CHECK(!ps2::gs::aspect2d::correct(candidate));

    c.test = 0;
    CHECK(!ps2::gs::aspect2d::shadow_like(candidate));
    CHECK(ps2::gs::aspect2d::classify(candidate) == DrawClass::Interface);
}

void hud() {
    constexpr std::uint32_t width = 640;
    const float ratios[] = {4.0f / 3.0f, 16.0f / 9.0f, 16.0f / 10.0f, 21.0f / 9.0f, 32.0f / 9.0f};
    for (const float ratio : ratios) {
        const float factor = (4.0f / 3.0f) / ratio;
        const std::uint32_t left = ps2::gs::aspect2d::corrected_x(0, 0, width, factor);
        const std::uint32_t right = ps2::gs::aspect2d::corrected_x(width * 16, 0, width, factor);
        CHECK(left + right == width * 16);
        CHECK(left <= right);
    }
    const float wide = (4.0f / 3.0f) / (16.0f / 9.0f);
    CHECK(ps2::gs::aspect2d::corrected_x(0, 0, width, wide) == 80u * 16u);
    CHECK(ps2::gs::aspect2d::corrected_x(width * 16, 0, width, wide) == 560u * 16u);

    Context c = context();
    c.zbuf = 1ull << 32;
    // Real draw logs print the raw TEX0.TBP0 field. Keep the fixture in those
    // units so a byte/page conversion mismatch cannot make this test pass.
    c.tex0 = 0x37a0u;
    Vertex position[2]{};
    position[0].x = 22 * 16; position[0].y = 25 * 16; position[0].z = 0x00ffff28u;
    position[1].x = 151 * 16; position[1].y = 202 * 16; position[1].z = 0x00ffff28u;
    const Signature left{6, 0x10u | 0x40u, &c, position, 2};
    CHECK(ps2::gs::hud::identify(left) == ps2::gs::hud::Group::LeftRace);
    position[0].x = 486 * 16; position[0].y = 19 * 16;
    position[1].x = 616 * 16; position[1].y = 181 * 16;
    CHECK(ps2::gs::hud::identify(left) == ps2::gs::hud::Group::RightRace);
    for (const std::uint32_t tbp : {0x2b00u, 0x2c00u, 0x2e00u, 0x37a0u, 0x38a0u, 0x3aa0u}) {
        c.tex0 = tbp;
        CHECK(ps2::gs::hud::texture_base_pointer(c) == tbp);
        CHECK(ps2::gs::hud::identify(left) == ps2::gs::hud::Group::RightRace);
    }
    // Captured PAL frame 4580: the dial/tachometer/needle/auxiliary gauge use
    // the dedicated 39a0 page as a 43-triangle group. Test two representative
    // triangles at opposite extents, not a synthetic broad bounding box.
    Vertex dial[3]{};
    dial[0].x = 417 * 16; dial[0].y = 358 * 16; dial[0].z = 0x001c71aeu; dial[0].q = 0.2f;
    dial[1].x = 417 * 16; dial[1].y = 369 * 16; dial[1].z = 0x001c71aeu; dial[1].q = 0.2f;
    dial[2].x = 444 * 16; dial[2].y = 358 * 16; dial[2].z = 0x001c71aeu; dial[2].q = 0.2f;
    c.tex0 = 0x39a0u;
    const Signature speed_mesh{4, 0x8u | 0x10u | 0x20u | 0x40u, &c, dial, 3};
    CHECK(ps2::gs::hud::identify(speed_mesh) == ps2::gs::hud::Group::Speedometer);
    dial[0].x = 370 * 16; dial[0].y = 426 * 16;
    dial[1].x = 417 * 16; dial[1].y = 369 * 16;
    dial[2].x = 370 * 16; dial[2].y = 369 * 16;
    CHECK(ps2::gs::hud::identify(speed_mesh) == ps2::gs::hud::Group::Speedometer);
    // Animated needle/tachometer and upper auxiliary-gauge triangles stay in
    // the same group even though neither reaches the cluster's outer bounds.
    dial[0].x = 540 * 16; dial[0].y = 343 * 16;
    dial[1].x = 527 * 16; dial[1].y = 349 * 16;
    dial[2].x = 537 * 16; dial[2].y = 351 * 16;
    CHECK(ps2::gs::hud::identify(speed_mesh) == ps2::gs::hud::Group::Speedometer);
    dial[0].x = 516 * 16; dial[0].y = 290 * 16;
    dial[1].x = 516 * 16; dial[1].y = 247 * 16;
    dial[2].x = 506 * 16; dial[2].y = 291 * 16;
    CHECK(ps2::gs::hud::identify(speed_mesh) == ps2::gs::hud::Group::Speedometer);
    c.tex0 = 0x2d00u;
    CHECK(ps2::gs::hud::identify(speed_mesh) == ps2::gs::hud::Group::Speedometer);

    // The following shared-atlas sprites are the gear, digital speed and
    // associated label. Nearby map and centred-message sprites remain out.
    c.tex0 = 0x37a0u;
    position[0].x = 428 * 16; position[0].y = 365 * 16;
    position[1].x = 452 * 16; position[1].y = 378 * 16;
    CHECK(ps2::gs::hud::identify(left) == ps2::gs::hud::Group::Speedometer);
    position[0].x = 375 * 16; position[0].y = 386 * 16;
    position[1].x = 479 * 16; position[1].y = 422 * 16;
    CHECK(ps2::gs::hud::identify(left) == ps2::gs::hud::Group::Speedometer);
    c.tex0 = 0x2b00u;
    CHECK(ps2::gs::hud::identify(left) == ps2::gs::hud::Group::Speedometer);
    c.tex0 = 0x38a0u;
    CHECK(ps2::gs::hud::identify(left) == ps2::gs::hud::Group::None);
    c.tex0 = 0x37a0u;
    position[0].x = 220 * 16; position[0].y = 100 * 16;
    position[1].x = 420 * 16; position[1].y = 150 * 16;
    CHECK(ps2::gs::hud::identify(left) == ps2::gs::hud::Group::None);

    c.tex0 = 0x39a0u;
    dial[0].x = 220 * 16; dial[0].y = 100 * 16;
    dial[1].x = 320 * 16; dial[1].y = 150 * 16;
    dial[2].x = 420 * 16; dial[2].y = 100 * 16;
    CHECK(ps2::gs::hud::identify(speed_mesh) == ps2::gs::hud::Group::None);

    CHECK(ps2::gs::hud::edge_shift(ps2::gs::hud::Group::LeftRace, 640, 1.0f, 16) == 0);
    CHECK(ps2::gs::hud::edge_shift(ps2::gs::hud::Group::LeftRace, 640, wide, 16) == -64);
    CHECK(ps2::gs::hud::edge_shift(ps2::gs::hud::Group::RightRace, 640, wide, 16) == 64);
    CHECK(ps2::gs::hud::edge_shift(ps2::gs::hud::Group::Speedometer, 640, wide, 16) == 64);
    CHECK(ps2::gs::hud::shifted_x(80u * 16u, -64) == 16u * 16u);
    CHECK(ps2::gs::hud::edge_shift(ps2::gs::hud::Group::LeftRace, 640, (4.0f / 3.0f) / (16.0f / 10.0f), 16) == -37);
    CHECK(ps2::gs::hud::edge_shift(ps2::gs::hud::Group::RightRace, 640, (4.0f / 3.0f) / (16.0f / 10.0f), 16) == 37);
    for (const float ratio : ratios) {
        const float factor = (4.0f / 3.0f) / ratio;
        const int shift = ps2::gs::hud::edge_shift(ps2::gs::hud::Group::Speedometer, width, factor, 16);
        const std::uint32_t right = ps2::gs::aspect2d::corrected_x(612u * 16u, 0, width, factor);
        const std::uint32_t moved = ps2::gs::hud::shifted_x(right, shift);
        CHECK(moved <= (width - 1u) * 16u);
        CHECK((ratio == 4.0f / 3.0f) == (shift == 0));
    }
    CHECK(ps2::gs::hud::edge_shift(ps2::gs::hud::Group::Speedometer, 640, wide, 0) == 80);
    CHECK(ps2::gs::hud::edge_shift(ps2::gs::hud::Group::Speedometer, 640, wide, 32) == 48);
}

void scissor() {
    Context c = context();
    const float factor = (4.0f / 3.0f) / (16.0f / 9.0f);
    CHECK(ps2::gs::aspect2d::corrected_scissor(c.scissor, 640, factor) == c.scissor);

    const std::uint64_t partial = 1ull | (638ull << 16) | (4ull << 32) | (400ull << 48);
    const std::uint64_t moved = ps2::gs::aspect2d::corrected_scissor(partial, 640, factor);
    CHECK((moved & 0x7ffu) == 80u);
    CHECK(((moved >> 16) & 0x7ffu) == 559u);
    CHECK(((moved >> 32) & 0x7ffu) == 4u);
    CHECK(((moved >> 48) & 0x7ffu) == 400u);

    const std::uint64_t fractional = 2ull | (637ull << 16);
    const std::uint64_t fractional_moved = ps2::gs::aspect2d::corrected_scissor(fractional, 640, factor);
    CHECK((fractional_moved & 0x7ffu) == 81u);
    CHECK(((fractional_moved >> 16) & 0x7ffu) == 558u);

    const std::uint64_t clipped = 90ull | (140ull << 16) | (10ull << 32) | (100ull << 48);
    const std::uint64_t shifted = ps2::gs::hud::shifted_scissor(clipped, 640, -64);
    CHECK((shifted & 0x7ffu) == 26u && ((shifted >> 16) & 0x7ffu) == 76u);
    CHECK(ps2::gs::hud::shifted_scissor(c.scissor, 640, 64) == c.scissor);
}

void transitions() {
    Context c = context();
    c.zbuf = 1ull << 32;
    Vertex full[2]{};
    full[1].x = 640 * 16;
    full[1].y = 448 * 16;
    const Signature clear_or_effect{6, 0, &c, full, 2};
    CHECK(ps2::gs::aspect2d::classify(clear_or_effect) == DrawClass::FullSurface);
    CHECK(!ps2::gs::aspect2d::correct(clear_or_effect));

    Vertex ui[2]{};
    ui[0].x = 32 * 16; ui[0].y = 32 * 16; ui[0].z = 0x00ffff00u;
    ui[1].x = 128 * 16; ui[1].y = 96 * 16; ui[1].z = 0x00ffff00u;
    const Signature after{6, 0x10u, &c, ui, 2};
    CHECK(ps2::gs::aspect2d::correct(after));
    CHECK(ps2::gs::aspect2d::classify(after) == DrawClass::Interface);
}

void switching() {
    using namespace ps2;
    set_hud_layout(true, 16);
    CHECK(hud_edge_anchored() && hud_edge_margin() == 16);
    set_hud_layout(false, 28);
    CHECK(!hud_edge_anchored() && hud_edge_margin() == 28);
    CHECK(set_aspect("4:3") && !wide_aspect());
    CHECK(set_aspect("16:9") && std::fabs(display_aspect() - 16.0f / 9.0f) < 0.0001f);
    CHECK(set_aspect("16:10") && std::fabs(display_aspect() - 1.6f) < 0.0001f);
    CHECK(set_aspect("21:9") && std::fabs(display_aspect() - 21.0f / 9.0f) < 0.0001f);
    CHECK(set_aspect("32:9") && std::string(aspect_name(g_options.aspect)).find("experimental") != std::string::npos);
    CHECK(set_aspect("auto"));
    resolve_auto_aspect(2.0f);
    CHECK(std::fabs(display_aspect() - 2.0f) < 0.0001f);
    CHECK(!set_aspect("16:11"));
}

} // namespace

int main(int argc, char **argv) {
    if (argc != 2) {
        std::fprintf(stderr, "usage: ps2aspecttest classification|shadow|hud|scissor|transitions|switching\n");
        return 2;
    }
    const std::string test = argv[1];
    if (test == "classification") classification();
    else if (test == "shadow") shadow();
    else if (test == "hud") hud();
    else if (test == "scissor") scissor();
    else if (test == "transitions") transitions();
    else if (test == "switching") switching();
    else return 2;
    return failures == 0 ? 0 : 1;
}
