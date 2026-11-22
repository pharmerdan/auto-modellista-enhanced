# Auto Modellista Enhanced frontend

Phase 2B adds a host-side in-game settings frontend without changing game
physics, timing, the PAL widescreen projection patch, HUD classification,
car-shadow rendering, or scissor correction.

## Use

- `F10` opens or closes the enhanced menu. `F1` still opens the original quick
  menu.
- Up/Down or the controller D-pad/left stick selects a row. Left/Right changes
  a value. `Q`/`E` or `L1`/`R1` changes section. Enter/Space or Cross accepts;
  Escape/Backspace or Circle goes back. Select+L3 opens the enhanced menu from
  a controller.
- The game waits and receives neutral controller state while either menu is
  open. Input remains suppressed after closing until held buttons are released.
- `F3` toggles the compact performance overlay.

The six sections are Display, Graphics, Performance, Controls, Saves and
General. F5/F8 quick save/load and normal memory-card saves are unchanged.

Display aspect, fullscreen and picture style apply immediately. Resolution
scale, 2D filtering and VSync are marked `RESTART` because the renderer creates
their GPU resources or swap chain during startup. VSync appears only when the
active WebGPU surface offers both FIFO and Immediate presentation.

## Persistence and precedence

Settings are written automatically outside the game data directory:

- macOS: `~/Library/Application Support/ps2-native-recomp/automodellista-enhanced.ini`
- Windows: `%APPDATA%\ps2-native-recomp\automodellista-enhanced.ini`
- Linux: `$XDG_CONFIG_HOME/ps2-native-recomp/automodellista-enhanced.ini`, or
  `~/.config/ps2-native-recomp/automodellista-enhanced.ini`

`PS2_ENHANCED_SETTINGS` can name another settings file, mainly for tests and
portable launchers. Precedence is command-line flags, then `PS2_*` environment
variables, then this file, then defaults. Defaults retain the established
16:9, 2x Apple Silicon profile and the existing sharp/smoothed cel-shaded look.
Restore Defaults is available under General and asks for confirmation.

## Performance meanings

Presented FPS counts successful swap-chain presentations. Pictures the
renderer identifies as unchanged and skips do not increment it. Simulation FPS
is the emulated PAL/NTSC refresh counter per real second; Game speed compares
that value with 50 Hz PAL or 60 Hz NTSC. Frame time is real elapsed time divided
by actual presentations. The overlay therefore does not describe duplicated
screen presentations as extra game simulation.

## Renderer extensions

Phase 2C added single-pass CRT profiles, and Phase 2E added optional FidelityFX
CAS in the same presentation pass. See `automodellista-phase2c.md` and
`automodellista-phase2e.md`. Anisotropic filtering and FSR EASU remain
unimplemented and have no menu controls.

## Reference and licensing

The page-based navigation, explicit restart labelling, controller-first
navigation and stats terminology were informed by the documented F10 frontend
in [apexJCL/automodellista-recomp](https://github.com/apexJCL/automodellista-recomp),
which is MIT licensed. No Xbox game patches, assets, shader files, or source
code were copied; this implementation uses this project's existing CPU overlay
font and PS2 renderer interfaces.

## Build and launch

Release build:

```sh
cmake -S . -B build/native-automodellista-enhanced -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DPS2_GENERATED="$PWD/generated/automodellista-pal"
cmake --build build/native-automodellista-enhanced --target ps2run -j 8
```

Launch:

```sh
"$PWD/build/native-automodellista-enhanced/ps2run" \
  "$PWD/games/automodellista-pal/disc/SLES_511.91" \
  --game-dir "$PWD/games/automodellista-pal" \
  --window
```

Automated tests do not establish visual or gameplay correctness. Manually test
the title/menu transition, one race, HUD and car shadow at each aspect, F1/F10
input isolation, F3 metrics, fullscreen, quick saves, normal saves, and a
restart after changing each restart-marked option.
