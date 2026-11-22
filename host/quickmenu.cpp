#include "quickmenu.hpp"

#include <algorithm>
#include <cstring>

namespace quickmenu {
namespace {

// The font: 5 x 8 pixels per letter, printable ASCII (32 to 126), drawn for this project (no font file, no licence to carry). One letter a
// line, its 8 rows from the top separated by spaces, '#' a lit pixel. Capitals and digits use rows 0 to 6; row 7 is for the tails of
// g, j, p, q, y and the comma.
constexpr const char *kFont[95] = {
    "..... ..... ..... ..... ..... ..... ..... .....",  // space
    "..#.. ..#.. ..#.. ..#.. ..#.. ..... ..#.. .....",  // !
    ".#.#. .#.#. ..... ..... ..... ..... ..... .....",  // "
    ".#.#. .#.#. ##### .#.#. ##### .#.#. .#.#. .....",  // #
    "..#.. .#### #.#.. .###. ..#.# ####. ..#.. .....",  // $
    "##... ##..# ...#. ..#.. .#... #..## ...## .....",  // %
    ".##.. #..#. #.#.. .#... #.#.# #..#. .##.# .....",  // &
    "..#.. ..#.. ..... ..... ..... ..... ..... .....",  // '
    "...#. ..#.. .#... .#... .#... ..#.. ...#. .....",  // (
    ".#... ..#.. ...#. ...#. ...#. ..#.. .#... .....",  // )
    "..... ..#.. #.#.# .###. #.#.# ..#.. ..... .....",  // *
    "..... ..#.. ..#.. ##### ..#.. ..#.. ..... .....",  // +
    "..... ..... ..... ..... ..... .##.. ..#.. .#...",  // ,
    "..... ..... ..... ##### ..... ..... ..... .....",  // -
    "..... ..... ..... ..... ..... .##.. .##.. .....",  // .
    "..... ....# ...#. ..#.. .#... #.... ..... .....",  // /
    ".###. #...# #..## #.#.# ##..# #...# .###. .....",  // 0
    "..#.. .##.. ..#.. ..#.. ..#.. ..#.. .###. .....",  // 1
    ".###. #...# ....# ...#. ..#.. .#... ##### .....",  // 2
    "##### ...#. ..#.. ...#. ....# #...# .###. .....",  // 3
    "...#. ..##. .#.#. #..#. ##### ...#. ...#. .....",  // 4
    "##### #.... ####. ....# ....# #...# .###. .....",  // 5
    "..##. .#... #.... ####. #...# #...# .###. .....",  // 6
    "##### ....# ...#. ..#.. .#... .#... .#... .....",  // 7
    ".###. #...# #...# .###. #...# #...# .###. .....",  // 8
    ".###. #...# #...# .#### ....# ...#. .##.. .....",  // 9
    "..... .##.. .##.. ..... .##.. .##.. ..... .....",  // :
    "..... .##.. .##.. ..... .##.. ..#.. .#... .....",  // ;
    "...#. ..#.. .#... #.... .#... ..#.. ...#. .....",  // <
    "..... ..... ##### ..... ##### ..... ..... .....",  // =
    ".#... ..#.. ...#. ....# ...#. ..#.. .#... .....",  // >
    ".###. #...# ....# ...#. ..#.. ..... ..#.. .....",  // ?
    ".###. #...# ....# .##.# #.#.# #.#.# .###. .....",  // @
    ".###. #...# #...# ##### #...# #...# #...# .....",  // A
    "####. #...# #...# ####. #...# #...# ####. .....",  // B
    ".###. #...# #.... #.... #.... #...# .###. .....",  // C
    "####. #...# #...# #...# #...# #...# ####. .....",  // D
    "##### #.... #.... ####. #.... #.... ##### .....",  // E
    "##### #.... #.... ####. #.... #.... #.... .....",  // F
    ".###. #...# #.... #.### #...# #...# .#### .....",  // G
    "#...# #...# #...# ##### #...# #...# #...# .....",  // H
    ".###. ..#.. ..#.. ..#.. ..#.. ..#.. .###. .....",  // I
    "..### ...#. ...#. ...#. ...#. #..#. .##.. .....",  // J
    "#...# #..#. #.#.. ##... #.#.. #..#. #...# .....",  // K
    "#.... #.... #.... #.... #.... #.... ##### .....",  // L
    "#...# ##.## #.#.# #.#.# #...# #...# #...# .....",  // M
    "#...# #...# ##..# #.#.# #..## #...# #...# .....",  // N
    ".###. #...# #...# #...# #...# #...# .###. .....",  // O
    "####. #...# #...# ####. #.... #.... #.... .....",  // P
    ".###. #...# #...# #...# #.#.# #..#. .##.# .....",  // Q
    "####. #...# #...# ####. #.#.. #..#. #...# .....",  // R
    ".#### #.... #.... .###. ....# ....# ####. .....",  // S
    "##### ..#.. ..#.. ..#.. ..#.. ..#.. ..#.. .....",  // T
    "#...# #...# #...# #...# #...# #...# .###. .....",  // U
    "#...# #...# #...# #...# #...# .#.#. ..#.. .....",  // V
    "#...# #...# #...# #.#.# #.#.# #.#.# .#.#. .....",  // W
    "#...# #...# .#.#. ..#.. .#.#. #...# #...# .....",  // X
    "#...# #...# .#.#. ..#.. ..#.. ..#.. ..#.. .....",  // Y
    "##### ....# ...#. ..#.. .#... #.... ##### .....",  // Z
    ".###. .#... .#... .#... .#... .#... .###. .....",  // [
    "..... #.... .#... ..#.. ...#. ....# ..... .....",  // backslash
    ".###. ...#. ...#. ...#. ...#. ...#. .###. .....",  // ]
    "..#.. .#.#. #...# ..... ..... ..... ..... .....",  // ^
    "..... ..... ..... ..... ..... ..... ##### .....",  // _
    ".#... ..#.. ..... ..... ..... ..... ..... .....",  // `
    "..... ..... .###. ....# .#### #...# .#### .....",  // a
    "#.... #.... #.##. ##..# #...# #...# ####. .....",  // b
    "..... ..... .###. #.... #.... #...# .###. .....",  // c
    "....# ....# .##.# #..## #...# #...# .#### .....",  // d
    "..... ..... .###. #...# ##### #.... .###. .....",  // e
    "..##. .#..# .#... ###.. .#... .#... .#... .....",  // f
    "..... ..... .#### #...# #...# .#### ....# .###.",  // g
    "#.... #.... #.##. ##..# #...# #...# #...# .....",  // h
    "..#.. ..... .##.. ..#.. ..#.. ..#.. .###. .....",  // i
    "...#. ..... ..##. ...#. ...#. ...#. #..#. .##..",  // j
    "#.... #.... #..#. #.#.. ##... #.#.. #..#. .....",  // k
    ".##.. ..#.. ..#.. ..#.. ..#.. ..#.. .###. .....",  // l
    "..... ..... ##.#. #.#.# #.#.# #.#.# #.#.# .....",  // m
    "..... ..... #.##. ##..# #...# #...# #...# .....",  // n
    "..... ..... .###. #...# #...# #...# .###. .....",  // o
    "..... ..... ####. #...# #...# ####. #.... #....",  // p
    "..... ..... .#### #...# #...# .#### ....# ....#",  // q
    "..... ..... #.##. ##..# #.... #.... #.... .....",  // r
    "..... ..... .###. #.... .###. ....# ####. .....",  // s
    ".#... .#... ###.. .#... .#... .#..# ..##. .....",  // t
    "..... ..... #...# #...# #...# #..## .##.# .....",  // u
    "..... ..... #...# #...# #...# .#.#. ..#.. .....",  // v
    "..... ..... #...# #...# #.#.# #.#.# .#.#. .....",  // w
    "..... ..... #...# .#.#. ..#.. .#.#. #...# .....",  // x
    "..... ..... #...# #...# #...# .#### ....# .###.",  // y
    "..... ..... ##### ...#. ..#.. .#... ##### .....",  // z
    "...#. ..#.. ..#.. .#... ..#.. ..#.. ...#. .....",  // {
    "..#.. ..#.. ..#.. ..#.. ..#.. ..#.. ..#.. .....",  // |
    ".#... ..#.. ..#.. ...#. ..#.. ..#.. .#... .....",  // }
    "..... ..... .#... #.#.# ...#. ..... ..... .....",  // ~
};

constexpr int kEntries = 10;
constexpr Command kCommands[kEntries] = {Command::resume,   Command::save,       Command::load, Command::look,        Command::scale,
                                         Command::sharp_2d, Command::fullscreen, Command::keys, Command::change_keys, Command::quit};
// the program's key for the same thing, shown dim at the right end of the entry
constexpr const char *kKeys[kEntries] = {"F1", "F5", "F8", "F2", "", "", "F11", "", "", ""};
constexpr int kVisibleRows = 11;  // lines of the keys page shown at once; the page scrolls
constexpr int kKeysPageChars = 58;

constexpr std::uint32_t rgba(int r, int g, int b, int a) {
    return static_cast<std::uint32_t>(r) | static_cast<std::uint32_t>(g) << 8 | static_cast<std::uint32_t>(b) << 16 | static_cast<std::uint32_t>(a) << 24;
}
constexpr std::uint32_t kPanel = rgba(12, 14, 20, 215), kFrame = rgba(96, 104, 124, 255), kTitle = rgba(240, 200, 110, 255),
                        kText = rgba(226, 229, 236, 255), kBar = rgba(236, 190, 88, 248), kBarText = rgba(16, 16, 20, 255),
                        kDim = rgba(140, 146, 162, 255), kBarDim = rgba(84, 62, 24, 255);

struct Canvas {
    Picture &p;
    void fill(int x, int y, int w, int h, std::uint32_t color) {
        const int x0 = std::max(0, x), y0 = std::max(0, y), x1 = std::min(p.width, x + w), y1 = std::min(p.height, y + h);
        for (int j = y0; j < y1; ++j) std::fill(p.rgba.begin() + j * p.width + x0, p.rgba.begin() + j * p.width + x1, color);
    }
    // one letter is 5 x 8 pixels of the font, each `s` x `s` screen pixels; a letter is 6 font pixels wide with its gap
    void text(int x, int y, int s, const std::string &line, std::uint32_t color) {
        for (unsigned char c : line) {
            const char *glyph = kFont[(c >= 32 && c <= 126 ? c : '?') - 32];
            for (int row = 0; row < 8; ++row)
                for (int col = 0; col < 5; ++col)
                    if (glyph[row * 6 + col] == '#') fill(x + col * s, y + row * s, s, s, color);
            x += 6 * s;
        }
    }
};

std::string entry_text(int i, const Values &v, bool quit_armed) {
    switch (kCommands[i]) {
    case Command::resume: return "Resume";
    case Command::save: return "Quick save";
    case Command::load: return "Quick load";
    case Command::look: return "Picture look:  < " + v.look + " >";
    case Command::scale: {
        const std::string now = std::to_string(v.scale_now) + "x";
        if (v.scale_locked) return "Resolution: " + now + " (set by PS2_SCALE)";
        return "Resolution:  < " + std::to_string(v.scale_next) + "x >" + (v.scale_next != v.scale_now ? "  at the next start" : "");
    }
    case Command::sharp_2d: {
        static const char *const kNames[] = {"soft", "sharp", "sharpest"};
        const auto name = [](int n) { return kNames[n >= 0 && n <= 2 ? n : 1]; };
        if (v.sharp_locked) return std::string("Text and menus: ") + name(v.sharp_now) + " (set by PS2_SHARP_2D)";
        return std::string("Text and menus:  < ") + name(v.sharp_next) + " >" + (v.sharp_next != v.sharp_now ? "  at the next start" : "");
    }
    case Command::fullscreen: return std::string("Full screen: ") + (v.fullscreen ? "on" : "off");
    case Command::keys: return "Show keys";
    case Command::change_keys: return "Change keys";
    case Command::quit: return quit_armed ? "Really quit?  Enter / Cross again" : "Quit the game";
    default: return "";
    }
}

}  // namespace

int Menu::count() const {
    if (page_ == Page::keys) return key_rows_ + 2;
    if (page_ == Page::main) return kEntries;
    switch (section_) {
    case Section::display: return 6;
    case Section::graphics: return 12 + (vsync_available_ ? 1 : 0);
    case Section::performance: return 1;
    case Section::controls: return 2;
    case Section::saves: return 2;
    case Section::general: return 2;
    }
    return 1;
}

void Menu::fit() {
    const int shown = std::min(key_rows_, kVisibleRows);
    if (selected_ < key_rows_) {
        if (selected_ < top_) top_ = selected_;
        if (selected_ >= top_ + shown) top_ = selected_ - shown + 1;
    }
    top_ = std::clamp(top_, 0, std::max(0, key_rows_ - shown));
}

bool Menu::back() {
    if (page_ != Page::keys) return false;
    page_ = parent_page_;
    selected_ = main_selected_;
    capturing_ = false;
    defaults_armed_ = false;
    return true;
}

void Menu::open() {
    open_ = true;
    quit_armed_ = false;
    page_ = Page::main;
    capturing_ = false;
    defaults_armed_ = false;
    top_ = 0;
    selected_ = 0;  // always starts on Resume, so a second press of the key that opened it gives the game back
}

void Menu::open_enhanced() {
    open_ = true;
    page_ = Page::enhanced;
    parent_page_ = Page::enhanced;
    section_ = Section::display;
    selected_ = 0;
    capturing_ = false;
    defaults_armed_ = false;
    settings_defaults_armed_ = false;
    quit_armed_ = false;
}

void Menu::move(int delta) {
    quit_armed_ = false;
    defaults_armed_ = false;
    settings_defaults_armed_ = false;
    const int n = count();
    selected_ = (selected_ + delta % n + n) % n;
    if (page_ == Page::keys) fit();
}

void Menu::section_move(int delta) {
    if (page_ != Page::enhanced) return;
    constexpr int sections = 6;
    section_ = static_cast<Section>((static_cast<int>(section_) + delta % sections + sections) % sections);
    selected_ = 0;
    settings_defaults_armed_ = false;
}

Command Menu::choose() {
    if (page_ == Page::keys) {
        if (selected_ < key_rows_) {
            capturing_ = true;
            return Command::key_edit;
        }
        if (selected_ == key_rows_) {
            if (!defaults_armed_) {
                defaults_armed_ = true;  // asks once
                return Command::none;
            }
            defaults_armed_ = false;
            return Command::key_defaults;
        }
        back();
        return Command::none;
    }
    Command c = Command::none;
    if (page_ == Page::enhanced) {
        switch (section_) {
        case Section::display: {
            constexpr Command commands[] = {Command::aspect, Command::scale, Command::fullscreen, Command::hud_mode, Command::hud_margin, Command::reset_hud};
            c = commands[selected_];
            break;
        }
        case Section::graphics: {
            constexpr Command commands[] = {Command::look, Command::sharp_2d, Command::image_filter, Command::sharpening, Command::reset_image,
                                            Command::crt_profile, Command::crt_scanline, Command::crt_mask,
                                            Command::crt_curvature, Command::crt_brightness, Command::crt_sharpness, Command::reset_crt, Command::vsync};
            c = commands[selected_];
            break;
        }
        case Section::performance: c = Command::performance_overlay; break;
        case Section::controls: c = selected_ == 0 ? Command::keys : Command::change_keys; break;
        case Section::saves: c = selected_ == 0 ? Command::save : Command::load; break;
        case Section::general:
            c = selected_ == 0 ? Command::restore_defaults : Command::resume;
            if (c == Command::restore_defaults && !settings_defaults_armed_) {
                settings_defaults_armed_ = true;
                return Command::none;
            }
            settings_defaults_armed_ = false;
            break;
        }
    } else {
        c = kCommands[selected_];
    }
    if (c == Command::change_keys) {
        main_selected_ = selected_;
        parent_page_ = page_;
        page_ = Page::keys;
        selected_ = 0;
        top_ = 0;
        capturing_ = false;
        defaults_armed_ = false;
        return c;
    }
    if (c != Command::quit) return c;
    if (quit_armed_) return c;
    quit_armed_ = true;  // asks once
    return Command::none;
}

Command Menu::side() const {
    if (page_ == Page::keys) return Command::none;
    if (page_ == Page::enhanced) {
        switch (section_) {
        case Section::display: {
            constexpr Command commands[] = {Command::aspect, Command::scale, Command::fullscreen, Command::hud_mode, Command::hud_margin, Command::reset_hud};
            return commands[selected_];
        }
        case Section::graphics: {
            constexpr Command commands[] = {Command::look, Command::sharp_2d, Command::image_filter, Command::sharpening, Command::reset_image,
                                            Command::crt_profile, Command::crt_scanline, Command::crt_mask,
                                            Command::crt_curvature, Command::crt_brightness, Command::crt_sharpness, Command::reset_crt, Command::vsync};
            return commands[selected_];
        }
        case Section::performance: return Command::performance_overlay;
        default: return Command::none;
        }
    }
    const Command c = kCommands[selected_];
    return c == Command::look || c == Command::scale || c == Command::sharp_2d || c == Command::fullscreen ? c : Command::none;
}

bool Menu::select_at(int x, int y) {
    if (page_ == Page::keys) {
        if (capturing_ || entry_h_ <= 0 || x < entry_x_ || x >= entry_x_ + entry_w_) return false;
        int now = -1;
        if (y >= rows_y_ && y < rows_y_ + keys_shown_ * entry_h_) now = top_ + (y - rows_y_) / entry_h_;
        else if (y >= footer_y_ && y < footer_y_ + 2 * entry_h_) now = key_rows_ + (y - footer_y_) / entry_h_;
        if (now < 0) return false;
        if (now != selected_) defaults_armed_ = false;
        selected_ = now;
        return true;
    }
    if (entry_h_ <= 0 || x < entry_x_ || x >= entry_x_ + entry_w_ || y < entry_y_ || y >= entry_y_ + count() * entry_h_) return false;
    const int now = (y - entry_y_) / entry_h_;
    if (now != selected_) quit_armed_ = false;
    selected_ = now;
    return true;
}

void Menu::draw_keys(int window_width, int window_height, const Values &values, Picture &out) {
    static const std::string kTitleText = "Change keys - the game waits", kHint = "Enter / click: change a key     Esc / Circle: back",
                             kFixed = "Not usable: F1 F2 F3 F5 F8 F9 F10 F11 (program keys)", kCapture = "Press a key or mouse button... (Esc: cancel)";
    key_rows_ = static_cast<int>(values.key_rows.size());
    fit();
    const int n = key_rows_, shown = std::min(n, kVisibleRows);
    keys_shown_ = shown;
    const int pad = 8, bar_h = 12, bar_pad = 5, label_chars = 18, keys_chars = 30;
    const int units_w = 2 * pad + 2 * bar_pad + kKeysPageChars * 6;
    const int units_h = pad + 8 + 12 + shown * bar_h + 3 + 2 * bar_h + 11 + 12 + 12 + 8 + pad;
    const int s = std::clamp(std::min(window_height * 4 / 5 / units_h, window_width * 23 / 25 / units_w), 1, 6);
    out.width = units_w * s;
    out.height = units_h * s;
    out.x = std::max(0, (window_width - out.width) / 2);
    out.y = std::max(0, (window_height - out.height) / 2);
    out.rgba.assign(static_cast<std::size_t>(out.width) * out.height, kPanel);
    Canvas c{out};
    const int frame = std::max(1, s / 2);
    c.fill(0, 0, out.width, frame, kFrame);
    c.fill(0, out.height - frame, out.width, frame, kFrame);
    c.fill(0, 0, frame, out.height, kFrame);
    c.fill(out.width - frame, 0, frame, out.height, kFrame);
    const auto fit_text = [](const std::string &t, std::size_t max) { return t.size() <= max ? t : t.substr(0, max - 3) + "..."; };
    const int text_x = (pad + bar_pad) * s, inner_w = out.width - 2 * pad * s;
    int y = pad * s;
    c.text(text_x, y, s, kTitleText, kTitle);
    if (n > shown) {
        const std::string where = std::to_string(top_ + 1) + "-" + std::to_string(top_ + shown) + " of " + std::to_string(n);
        c.text(out.width - text_x - static_cast<int>(where.size()) * 6 * s + s, y, s, where, kDim);
    }
    y += 8 * s + 6 * s;
    c.fill(pad * s, y, inner_w, std::max(1, s / 2), kFrame);
    y += 6 * s;
    entry_x_ = out.x + pad * s;
    entry_w_ = inner_w;
    entry_h_ = bar_h * s;
    rows_y_ = out.y + y;
    const auto line = [&](int index, const std::string &left, const std::string &right) {
        const bool lit = index == selected_;
        if (lit) c.fill(pad * s, y, inner_w, bar_h * s, kBar);
        const std::uint32_t color = lit ? kBarText : kText;
        c.text(text_x, y + 2 * s, s, left, color);
        if (!right.empty()) c.text(text_x + (label_chars + 1) * 6 * s, y + 2 * s, s, fit_text(right, keys_chars), color);
        y += bar_h * s;
    };
    for (int r = 0; r < shown; ++r) {
        const int index = top_ + r;
        const Values::KeyRow &row = values.key_rows[index];
        if (index == selected_ && capturing_) line(index, kCapture, "");
        else line(index, row.label, row.keys);
    }
    y += 3 * s;
    footer_y_ = out.y + y;
    line(n, defaults_armed_ ? "Really put back the default keys?  Enter / Cross again" : "Put back the default keys", "");
    line(n + 1, "Back", "");
    y += 5 * s;
    c.fill(pad * s, y, inner_w, std::max(1, s / 2), kFrame);
    y += 6 * s;
    c.text(text_x, y, s, fit_text(values.key_note, kKeysPageChars), kTitle);
    y += 12 * s;
    c.text(text_x, y, s, kHint, kDim);
    y += 12 * s;
    c.text(text_x, y, s, kFixed, kDim);
}

void Menu::draw_enhanced(int window_width, int window_height, const Values &v, Picture &out) {
    static const char *const section_names[] = {"Display", "Graphics", "Performance", "Controls", "Saves", "General"};
    vsync_available_ = v.vsync_available;
    selected_ = std::min(selected_, count() - 1);
    std::vector<std::string> lines;
    std::vector<std::string> details;
    switch (section_) {
    case Section::display:
        lines = {v.aspect_locked ? "Aspect ratio: " + v.aspect + " (CLI / PS2_ASPECT)" : "Aspect ratio:  < " + v.aspect + " >",
                 v.scale_locked ? "Resolution scale: " + std::to_string(v.scale_now) + "x (PS2_SCALE)"
                                : "Resolution scale:  < " + std::to_string(v.scale_next) + "x >" + (v.scale_next == v.scale_now ? "" : "  RESTART"),
                 std::string("Fullscreen:  < ") + (v.fullscreen ? "on" : "off") + " >",
                 std::string("HUD positioning:  < ") + (v.hud_edge_anchored ? "widescreen edges" : "original 4:3-safe") + " >",
                 "HUD edge margin:  < " + std::to_string(v.hud_margin) + " px >", "Reset HUD layout"};
        details = {"Auto and fixed PAL multi-aspect modes apply immediately.", "HUD edge mode moves only positively identified PAL race-HUD draws.",
                   "Pause, messages and speedometer remain in their original positions."};
        break;
    case Section::graphics:
        lines = {"Picture style / scaling:  < " + v.look + " >",
                 std::string("Text and menus:  < ") + (v.sharp_next == 0 ? "soft" : v.sharp_next == 1 ? "sharp" : "sharpest") + " >" +
                     (v.sharp_next == v.sharp_now ? "" : "  RESTART"),
                 v.image_filter_locked ? "Image filter: " + v.image_filter + " (CLI / PS2_IMAGE_FILTER)"
                                       : "Image filter:  < " + v.image_filter + " >",
                 v.sharpening_locked ? "Sharpening strength: " + std::to_string(v.sharpening) + "% (CLI / PS2_SHARPENING)"
                                      : "Sharpening strength:  < " + std::to_string(v.sharpening) + "% >",
                 "Restore image defaults",
                 "CRT profile:  < " + v.crt_profile + " >",
                 "Scanline intensity:  < " + std::to_string(v.crt_scanline) + "% >",
                 "Shadow mask strength:  < " + std::to_string(v.crt_mask) + "% >",
                 "Screen curvature:  < " + std::to_string(v.crt_curvature) + "% >",
                 "Brightness compensation:  < " + std::to_string(v.crt_brightness) + "% >",
                 "Image sharpness:  < " + std::to_string(v.crt_sharpness) + "% >", "Reset CRT settings"};
        if (v.vsync_available)
            lines.push_back(v.vsync_locked ? std::string("VSync: ") + (v.vsync ? "on" : "off") + " (PS2_PRESENT_MODE)"
                                           : std::string("VSync:  < ") + (v.vsync ? "on" : "off") + " >  RESTART");
        details = {"Original preserves the existing scaler; CAS is live, single-pass and does not change internal resolution.",
                   "Order: scale then CAS; with CRT, CAS source enhancement then CRT reconstruction.",
                   "F10, F1 and F3 overlays are composited afterwards and are never sharpened."};
        break;
    case Section::performance:
        lines = {std::string("F3 performance overlay:  < ") + (v.performance_overlay ? "on" : "off") + " >"};
        details = {"Presented: " + v.presented_fps + " fps    Frame time: " + v.frame_time + " ms",
                   "Simulation: " + v.simulation_fps + " fps    Game speed: " + v.game_speed,
                   "Internal: " + std::to_string(v.scale_now) + "x    Aspect: " + v.aspect,
                   "Presented FPS counts actual swap-chain presentations, not simulation steps."};
        break;
    case Section::controls:
        lines = {"Show current keyboard mappings", "Change keyboard mappings"};
        details = {"D-pad / left stick navigates; Cross accepts; Circle goes back.", "Game input is suppressed until menu buttons are released."};
        break;
    case Section::saves:
        lines = {"Quick save  (F5)", "Quick load  (F8)"};
        details = {"Normal memory-card saves and save-state formats are unchanged."};
        break;
    case Section::general:
        lines = {settings_defaults_armed_ ? "Really restore defaults?  Enter / Cross again" : "Restore enhanced defaults", "Resume game"};
        details = {"Settings save automatically.", "File: " + v.settings_path};
        break;
    }

    const std::string title = "AUTO MODELLISTA ENHANCED";
    const std::string hint = "Q/E or L1/R1: section   Arrows / D-pad: navigate   F10/Esc/Circle: close";
    std::string tabs;
    for (int i = 0; i < 6; ++i) tabs += std::string(i ? "  " : "") + (i == static_cast<int>(section_) ? "[" : "") + section_names[i] + (i == static_cast<int>(section_) ? "]" : "");
    std::size_t chars = std::max({title.size(), tabs.size(), hint.size(), std::size_t{68}});
    for (const auto &line : lines) chars = std::max(chars, line.size());
    for (const auto &line : details) chars = std::max(chars, std::min<std::size_t>(line.size(), 78));
    const int pad = 8, bar_h = 14, bar_pad = 5, detail_h = 11;
    const int units_w = 2 * pad + 2 * bar_pad + static_cast<int>(chars) * 6;
    const int units_h = pad + 8 + 11 + 8 + 10 + static_cast<int>(lines.size()) * bar_h + 8 + static_cast<int>(details.size()) * detail_h + 9 + 8 + pad;
    const int s = std::clamp(std::min(window_height * 9 / 10 / units_h, window_width * 24 / 25 / units_w), 1, 6);
    out.width = units_w * s;
    out.height = units_h * s;
    out.x = std::max(0, (window_width - out.width) / 2);
    out.y = std::max(0, (window_height - out.height) / 2);
    out.rgba.assign(static_cast<std::size_t>(out.width) * out.height, kPanel);
    Canvas c{out};
    const int frame = std::max(1, s / 2), inner_w = out.width - 2 * pad * s, text_x = (pad + bar_pad) * s;
    c.fill(0, 0, out.width, frame, kFrame); c.fill(0, out.height - frame, out.width, frame, kFrame);
    c.fill(0, 0, frame, out.height, kFrame); c.fill(out.width - frame, 0, frame, out.height, kFrame);
    int y = pad * s;
    c.text(text_x, y, s, title, kTitle); y += 11 * s;
    c.text(text_x, y, s, tabs, kText); y += 11 * s;
    c.fill(pad * s, y, inner_w, std::max(1, s / 2), kFrame); y += 6 * s;
    entry_x_ = out.x + pad * s; entry_y_ = out.y + y; entry_w_ = inner_w; entry_h_ = bar_h * s;
    for (int i = 0; i < static_cast<int>(lines.size()); ++i) {
        const bool lit = i == selected_;
        if (lit) c.fill(pad * s, y, inner_w, bar_h * s, kBar);
        c.text(text_x, y + 3 * s, s, lines[i], lit ? kBarText : kText);
        y += bar_h * s;
    }
    y += 3 * s; c.fill(pad * s, y, inner_w, std::max(1, s / 2), kFrame); y += 6 * s;
    for (std::string line : details) {
        if (line.size() > chars) line = line.substr(0, chars - 3) + "...";
        c.text(text_x, y, s, line, kDim); y += detail_h * s;
    }
    c.text(text_x, out.height - (pad + 8) * s, s, hint, kDim);
}

void Menu::draw(int window_width, int window_height, const Values &values, Picture &out) {
    if (page_ == Page::keys) {
        draw_keys(window_width, window_height, values, out);
        return;
    }
    if (page_ == Page::enhanced) {
        draw_enhanced(window_width, window_height, values, out);
        return;
    }
    static const std::string kTitleText = "Quick menu - the game waits", kHint = "Enter / Cross: OK     F1 / Esc / Circle: back";
    std::string lines[kEntries];
    std::size_t chars = std::max(kTitleText.size(), kHint.size());
    for (int i = 0; i < kEntries; ++i) {
        lines[i] = entry_text(i, values, quit_armed_);
        chars = std::max(chars, lines[i].size() + 5);  // room for the dim key at the right end
    }
    // everything in font pixels first; one font pixel is `s` window pixels, as big as fits in about 80 % of the window
    const int pad = 8, bar_h = 14, bar_pad = 5;
    const int units_w = 2 * pad + 2 * bar_pad + static_cast<int>(chars) * 6;
    const int units_h = pad + 8 + 12 + kEntries * bar_h + 11 + 8 + pad;
    const int s = std::clamp(std::min(window_height * 4 / 5 / units_h, window_width * 23 / 25 / units_w), 1, 6);
    out.width = units_w * s;
    out.height = units_h * s;
    out.x = std::max(0, (window_width - out.width) / 2);
    out.y = std::max(0, (window_height - out.height) / 2);
    out.rgba.assign(static_cast<std::size_t>(out.width) * out.height, kPanel);
    Canvas c{out};
    const int frame = std::max(1, s / 2);
    c.fill(0, 0, out.width, frame, kFrame);
    c.fill(0, out.height - frame, out.width, frame, kFrame);
    c.fill(0, 0, frame, out.height, kFrame);
    c.fill(out.width - frame, 0, frame, out.height, kFrame);
    int y = pad * s;
    c.text((pad + bar_pad) * s, y, s, kTitleText, kTitle);
    y += 8 * s + 6 * s;
    c.fill(pad * s, y, out.width - 2 * pad * s, std::max(1, s / 2), kFrame);
    y += 6 * s;
    entry_x_ = out.x + pad * s;
    entry_y_ = out.y + y;
    entry_w_ = out.width - 2 * pad * s;
    entry_h_ = bar_h * s;
    for (int i = 0; i < kEntries; ++i) {
        const bool lit = i == selected_;
        if (lit) c.fill(pad * s, y, out.width - 2 * pad * s, bar_h * s, kBar);
        c.text((pad + bar_pad) * s, y + 3 * s, s, lines[i], lit ? kBarText : kText);
        const std::string key = kKeys[i];
        c.text(out.width - (pad + bar_pad) * s - static_cast<int>(key.size()) * 6 * s + s, y + 3 * s, s, key, lit ? kBarDim : kDim);
        y += bar_h * s;
    }
    y += 5 * s;
    c.fill(pad * s, y, out.width - 2 * pad * s, std::max(1, s / 2), kFrame);
    y += 6 * s;
    c.text((pad + bar_pad) * s, y, s, kHint, kDim);
}

void Menu::draw_note(int window_width, int window_height, const std::string &text, Picture &out) {
    const int pad = 5;
    const int units_w = 2 * pad + static_cast<int>(text.size()) * 6, units_h = 2 * pad + 8;
    const int s = std::clamp(std::min(window_height / 240, window_width * 9 / 10 / units_w), 1, 6);
    out.width = units_w * s;
    out.height = units_h * s;
    out.x = std::max(0, (window_width - out.width) / 2);
    out.y = std::max(0, window_height / 20);
    out.rgba.assign(static_cast<std::size_t>(out.width) * out.height, kPanel);
    Canvas c{out};
    const int frame = std::max(1, s / 2);
    c.fill(0, 0, out.width, frame, kFrame);
    c.fill(0, out.height - frame, out.width, frame, kFrame);
    c.fill(0, 0, frame, out.height, kFrame);
    c.fill(out.width - frame, 0, frame, out.height, kFrame);
    c.text(pad * s, pad * s, s, text, kTitle);
}

void Menu::draw_performance(int window_width, int window_height, const Values &v, Picture &out) {
    const std::vector<std::string> lines = {
        "Presented " + v.presented_fps + " fps   " + v.frame_time + " ms",
        "Simulation " + v.simulation_fps + " fps   Speed " + v.game_speed,
        "Internal " + std::to_string(v.scale_now) + "x   Aspect " + v.aspect};
    std::size_t chars = 0;
    for (const auto &line : lines) chars = std::max(chars, line.size());
    const int pad = 5, gap = 3, units_w = 2 * pad + static_cast<int>(chars) * 6, units_h = 2 * pad + static_cast<int>(lines.size()) * 8 + (static_cast<int>(lines.size()) - 1) * gap;
    const int s = std::clamp(std::min(window_height / 360, window_width / std::max(units_w * 3, 1)), 1, 4);
    out.width = units_w * s; out.height = units_h * s;
    out.x = std::max(0, window_width - out.width - window_width / 50);
    out.y = std::max(0, window_height / 40);
    out.rgba.assign(static_cast<std::size_t>(out.width) * out.height, rgba(8, 10, 15, 190));
    Canvas c{out};
    int y = pad * s;
    for (const auto &line : lines) { c.text(pad * s, y, s, line, kText); y += (8 + gap) * s; }
}

}  // namespace quickmenu
