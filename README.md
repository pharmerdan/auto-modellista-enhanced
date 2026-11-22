# Auto Modellista Enhanced

**Auto Modellista Enhanced** is a native, enhanced port of Capcom's PAL
PlayStation 2 release of *Auto Modellista* (`SLES-51191`). It currently targets
Apple Silicon macOS, with Windows support planned.

The project preserves the game's distinctive cel-shaded presentation while
adding modern display options, configurable post-processing, and a native
desktop runtime. This repository contains source code only; you must provide
your own legally obtained PAL game files.

## Features

The following enhancements are implemented, although validation across the
entire game, every display mode, and all hardware configurations is ongoing:

- Native Apple Silicon execution
- 4:3, 16:9, 16:10, and 21:9 aspect ratios, plus experimental 32:9 support
- Corrected HUD proportions, with optional widescreen HUD edge anchoring
- CRT-Lottes Fast, CRT-Lottes, and CRT-Hyllian post-processing
- AMD FidelityFX Contrast Adaptive Sharpening (CAS)
- Enhanced graphics settings through the controller-friendly F10 menu
- F3 performance overlay
- Controller and keyboard configuration
- Virtual PS2 memory-card support
- Quick saves and normal in-game saves
- Persistent graphics preferences
- Internal resolution scaling from 1x to 4x

An M1 Pro has been observed running the game at 2x internal resolution and
60 FPS. Higher internal resolutions can be slower, depending on the scene and
hardware. This is an observed result, not a guarantee of equivalent performance
throughout the game.

## Screenshots

### Title Screen and Car Selection

| Title Screen | Car Selection |
|:---:|:---:|
| ![Title Screen](docs/screenshots/title-screen.png) | ![Car Selection](docs/screenshots/car-select.png) |

### 16:10 Widescreen Gameplay with CRT Effects

![Auto Modellista Enhanced — 16:10 CRT gameplay](docs/screenshots/gameplay-crt-16-10.png)

### Enhanced Graphics Frontend

| Display and HUD Settings | Graphics and CRT Settings |
|:---:|:---:|
| ![Enhanced Display Menu](docs/screenshots/enhanced-menu-display.png) | ![Enhanced Graphics Menu](docs/screenshots/enhanced-menu-graphics.png) |

## Installation and requirements

You need an Apple Silicon Mac with Xcode Command Line Tools, Git, CMake, Ninja,
Python 3, SDL2 development files, and Bash 4 or newer. With
[Homebrew](https://brew.sh/), install the dependencies with:

```sh
brew install bash cmake ninja python sdl2
```

Prepare the native recompiler and its pinned wgpu-native dependency:

```sh
"$(brew --prefix)/bin/bash" scripts/setup.sh --native-only
```

Create an image from your own legally obtained PAL *Auto Modellista* disc, then
run the porting workflow:

```sh
"$(brew --prefix)/bin/bash" scripts/port.sh automodellista-pal \
  "/path/to/your/Auto Modellista.iso" --native-only
```

For development builds against an already generated local tree:

```sh
cmake -S . -B build/native-automodellista-enhanced -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON \
  -DPS2_GENERATED="$PWD/generated/automodellista-pal"
cmake --build build/native-automodellista-enhanced --target ps2run
ctest --test-dir build/native-automodellista-enhanced --output-on-failure
```

Launch **Auto Modellista Enhanced** with persistent PS2 memory-card support:

```bash
cd ~/Projects/ps2-native-recomp

mkdir -p "$HOME/Library/Application Support/AutoModellistaEnhanced"

./build/native-automodellista-speedometer/ps2run \
  games/automodellista-pal-ws/patched/SLES_511.91 \
  --disc games/automodellista-pal-ws/disc.iso \
  --game-dir games/automodellista-pal-ws \
  --memcard "$HOME/Library/Application Support/AutoModellistaEnhanced/memcard.ps2" \
  --window \
  --no-record
```

The game loads saved graphics, CRT, CAS, aspect-ratio and HUD preferences.

**Memory cards:** Always reuse the same memory-card path to preserve progress. In-game memory-card saves are separate from F5/F8 save states.

**Shortcuts:** F10 opens Enhanced settings, F3 shows performance statistics, and F11 toggles fullscreen.

The generated code, extracted executable, disc image, native build, memory card,
and save states remain local and are ignored by Git. Never commit or distribute
a game ISO, game executable, PlayStation 2 BIOS, memory-card data, generated
game code, or copyrighted game assets.

For controls and contributor validation notes, see the
[enhanced frontend documentation](docs/automodellista-enhanced-frontend.md).

## Development roadmap

- Standalone macOS application
- Windows x64 release
- Additional compatibility improvements
- Optional graphics renderer optimisation

Roadmap items are planned work, not currently supported release features.

## Credits and acknowledgements

**Project development:** pharmerdan

**AI-assisted development:** OpenAI ChatGPT and OpenAI Codex were used for
development planning, source-code implementation, debugging, and optimisation
assistance. pharmerdan provided project direction, made the project decisions,
and performed gameplay validation. This acknowledgement does not imply that
OpenAI sponsors, endorses, or maintains the project.

The enhanced frontend design was also informed by the MIT-licensed
[apexJCL/automodellista-recomp](https://github.com/apexJCL/automodellista-recomp).
No Xbox-specific code or game assets from that project are included here.

## Upstream acknowledgement

Auto Modellista Enhanced builds upon
[exoteric33/ps2-native-recomp](https://github.com/exoteric33/ps2-native-recomp),
which provides the PS2 static-recompilation framework and runtime. Refer to the
[upstream documentation](https://github.com/exoteric33/ps2-native-recomp#readme)
for its architecture, general usage, and broader compatibility information.
Upstream is itself based on
[Samir Nuri's ps2-web-recomp](https://github.com/snuri00/ps2-web-recomp).

The original MIT copyright and permission notices are preserved in
[LICENSE](LICENSE). The MPEG decoder's BSD notice is preserved in
[runtime/src/ipu/NOTICE](runtime/src/ipu/NOTICE). Dependencies retain their own
licences and notices.

## Known limitations

- Some original frontend layout imperfections remain.
- Higher internal resolutions may not maintain full speed.
- 32:9 support is experimental.
- Compatibility and full-game validation are ongoing.

## Licence and trademarks

This project is available under the [MIT License](LICENSE), including the
preserved upstream copyright notices.

PlayStation and PS2 are trademarks of Sony Interactive Entertainment. *Auto
Modellista* and other game names are trademarks of their respective owners.
This project is not affiliated with or endorsed by Sony, Capcom, OpenAI, or any
other game publisher.

## Screenshots

### Title Screen
![Title Screen](docs/screenshots/title-screen.png)

### Car Selection
![Car Selection](docs/screenshots/car-select.png)

### 16:10 Gameplay with CRT Effects
![16:10 CRT Gameplay](docs/screenshots/gameplay-crt-16-10.png)

### Enhanced Menu — Display
![Display Settings](docs/screenshots/enhanced-menu-display.png)

### Enhanced Menu — Graphics
![Graphics Settings](docs/screenshots/enhanced-menu-graphics.png)
