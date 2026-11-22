# Auto Modellista Enhanced — Phase 2E

Phase 2E adds optional FidelityFX Contrast Adaptive Sharpening (CAS) to the
existing WebGPU presentation pass. It does not change the GS internal
resolution, guest timing, game geometry, HUD transforms, shadows, save data, or
controller handling. Original is the default and selects the pre-Phase-2E
presentation branches.

## Image processing

The Graphics page of F10 contains:

- Image filter: Original or CAS;
- Sharpening strength: 0–100%, in 5% steps;
- Restore image defaults.

CAS is implemented from AMD's published FidelityFX CAS algorithm: the soft 3x3
minimum/maximum, per-channel amplification and the documented negative-lobe
limit are retained. It is not labelled FSR because no EASU upscaler is present.
The filter runs in the existing render pass and allocates no intermediate
texture or other full-resolution GPU resource.

With CRT Off, processing is GS display reconstruction, the selected existing
scaler/picture style, then CAS. With CRT On, CAS operates on the source
neighbourhood before the selected CRT reconstruction. The latter is a more
expensive optional combination. F1, F3 and F10 are composited afterward in the
same render pass and are not processed. CRT is never silently disabled or
replaced.

Settings persist as `image_filter` and `sharpening`. Precedence is:

1. `--image-filter original|cas` and `--sharpening 0..100`;
2. `PS2_IMAGE_FILTER=original|cas` and `PS2_SHARPENING=0..100`;
3. the enhanced settings file;
4. Original and 35% defaults.

Forced values are labelled in F10 and cannot be overwritten there. Changes to
unforced values apply live.

## Renderer optimisation decision

The current compute renderer already contains tile/strip routing, block
uploads, row-range copies, unchanged-picture presentation skipping, asynchronous
submission, and timestamp-query profiling. Phase 2E does not add a speculative
GS fast path: no trustworthy windowed Metal comparison was available in the
build environment, and accuracy-critical blending, alpha/depth tests, texture
addressing, scissors and copy behaviour remain untouched.

Existing low-overhead diagnostics cover the requested areas:

- `PS2_GPU_PROFILE=FROM,TO,FILE` records timestamp-query duration for every GS
  compute dispatch and the presentation pass;
- `PS2_GPU_WGPU_TIME=300` reports CPU/wall time in uploads, copies, polling,
  waits, acquire, submit and present every 300 vblanks;
- `PS2_PRESENT_LOG=1` reports presentation hand-off and unchanged-picture skips;
- `scripts/gpu_profile.py FILE` summarises dispatch and presentation GPU time.

## Build and launch

```sh
cmake -S . -B build/native-automodellista-phase2e -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON \
  -DPS2_GENERATED="$PWD/generated/automodellista-pal"
cmake --build build/native-automodellista-phase2e --target ps2run -j 8
```

Exact patched-PAL launch command:

```sh
"$PWD/build/native-automodellista-phase2e/ps2run" \
  "$PWD/games/automodellista-pal-ws/patched/SLES_511.91" \
  --disc "$PWD/games/automodellista-pal-ws/disc.iso" \
  --game-dir "$PWD/games/automodellista-pal-ws" \
  --window
```

## Reproducible M1 Pro comparison

Use the same saved race, camera, window size and 600-frame interval for every
run. Set CRT and image filter in F10 before the measured interval. For a run
starting near frame 2400:

```sh
env PS2_SCALE=2 PS2_ASPECT=16:9 PS2_PRESENT_LOG=1 \
  PS2_GPU_WGPU_TIME=300 \
  PS2_GPU_PROFILE=2400,3000,/tmp/phase2e-2x.gpu \
  /usr/bin/time -l \
  "$PWD/build/native-automodellista-phase2e/ps2run" \
  "$PWD/games/automodellista-pal-ws/patched/SLES_511.91" \
  --disc "$PWD/games/automodellista-pal-ws/disc.iso" \
  --game-dir "$PWD/games/automodellista-pal-ws" \
  --window
python3 "$PWD/scripts/gpu_profile.py" /tmp/phase2e-2x.gpu
```

Repeat for 2x/CRT Off/Original, 2x/CRT-Lottes Fast/Original,
2x/CRT Off/CAS, and 3x/CRT Off/Original. Record the F3 Presented FPS, simulation
rate, game speed and frame time; the profile's `present` and total GPU times;
and `/usr/bin/time -l` maximum resident set size. A headless run is not a Metal
performance result.

## Manual correctness checklist

- At 2x/16:9, switch Original/CAS and move strength through 0, 35, 70 and 100%;
  inspect car outlines, road markings, fine text, gradients and haloing.
- Repeat with CRT Off and each CRT profile; confirm neither setting silently
  replaces the other and F1/F3/F10 stay sharp.
- Race at 4:3, 16:9, 16:10, 21:9 and experimental 32:9; inspect car shadows,
  screen boundaries, scenery, left/right HUD, centred messages and speedometer.
- Switch aspect, HUD anchoring and image filter live while steering/braking and
  changing camera; verify physics and simulation speed do not change.
- Exercise F5/F8, a normal memory-card save/load, controller mappings and a
  restart that reloads the image settings.
- Compare the four measured configurations above under identical conditions.

Automated and sandboxed tests do not establish visual correctness on a real
Metal display.
