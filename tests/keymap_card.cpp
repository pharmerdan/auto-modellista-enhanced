#include "keymap.hpp"

#include <SDL.h>

#include <cstdio>
#include <string>

namespace {

int failures = 0;

void check(bool ok, const char *what) {
    if (!ok) {
        std::fprintf(stderr, "FAIL: %s\n", what);
        ++failures;
    }
}

bool has(const std::string &text, const char *part) { return text.find(part) != std::string::npos; }

}  // namespace

int main() {
    keymap::Config config = keymap::defaults();
    config.inputs[keymap::Cross] = {SDL_SCANCODE_Z};
    config.inputs[keymap::Square] = {SDL_SCANCODE_B};

    const std::string racing = keymap::card_text(config, true);
    check(has(racing, "RACING CONTROLS"), "Auto Modellista card has racing section");
    check(has(racing, "Accelerate | Cross | Z"), "Auto Modellista card reads remapped accelerate key");
    check(has(racing, "Brake | Square | B"), "Auto Modellista card reads remapped brake key");
    check(has(racing, "Steering | Left stick / D-pad"), "Auto Modellista card identifies both steering controls");
    check(has(racing, "ENHANCED SHORTCUTS"), "Auto Modellista card has shortcut section");
    check(has(racing, "F10 Enhanced settings"), "Auto Modellista card lists enhanced settings");
    check(!has(racing, "Roll") && !has(racing, "Rage") && !has(racing, "magic") && !has(racing, "attack"),
          "Auto Modellista card has no action-game terminology");

    const std::string generic = keymap::card_text(config);
    check(has(generic, "Roll (evade)"), "generic card remains unchanged");

    return failures ? 1 : 0;
}
