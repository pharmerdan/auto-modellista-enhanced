#include "../host/quickmenu.hpp"

#include <cstdio>

int main() {
    int failed = 0;
    const auto check = [&](bool value, const char *name) {
        std::printf("%s %s\n", value ? "PASS" : "FAIL", name);
        failed += !value;
    };
    quickmenu::Menu menu;
    quickmenu::Values values;
    values.aspect = "16:9";
    values.scale_now = values.scale_next = 2;
    values.look = "Sharp + smooth edges";
    values.vsync_available = true;
    quickmenu::Picture picture;
    menu.open_enhanced();
    menu.draw(1280, 720, values, picture);
    check(menu.is_open() && menu.enhanced(), "enhanced menu opens independently");
    check(menu.choose() == quickmenu::Command::aspect, "Display starts on aspect ratio");
    menu.move(1);
    check(menu.choose() == quickmenu::Command::scale, "Display resolution row navigates");
    menu.section_move(1);
    check(menu.choose() == quickmenu::Command::look, "Graphics section opens on picture style");
    menu.move(2);
    check(menu.choose() == quickmenu::Command::image_filter, "Graphics exposes image filter selection");
    menu.move(1);
    check(menu.choose() == quickmenu::Command::sharpening, "Graphics exposes sharpening strength");
    menu.move(1);
    check(menu.choose() == quickmenu::Command::reset_image, "Graphics exposes image defaults");
    menu.move(8);
    check(menu.choose() == quickmenu::Command::vsync, "VSync appears only when renderer supports control");
    menu.section_move(1);
    check(menu.choose() == quickmenu::Command::performance_overlay, "Performance section toggles overlay");
    menu.draw(1280, 720, values, picture);
    check(picture.width > 0 && picture.height > 0 && !picture.rgba.empty(), "enhanced menu renders a panel");
    quickmenu::Menu::draw_performance(1280, 720, values, picture);
    check(picture.x > 0 && picture.width > 0 && !picture.rgba.empty(), "performance overlay renders in corner");
    return failed ? 1 : 0;
}
