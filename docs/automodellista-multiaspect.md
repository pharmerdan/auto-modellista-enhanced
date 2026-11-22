# Auto Modellista multi-aspect support

The native runner accepts `--aspect auto|4:3|16:9|16:10|21:9|32:9`.
`PS2_ASPECT` provides the same setting for launchers and configuration files
that expose environment variables. `--widescreen` remains a compatible alias
for 16:9.

For SLES-51191 the runtime updates the two projection values established by
the published PCSX2 widescreen patch. The display viewport uses the same ratio,
so 3D geometry is widened rather than stretched.

The GS dispatcher identifies Auto Modellista's 2D path from render state, not
merely primitive type. The game's high-Z, depth-masked overlay layer is
compressed about the 640-pixel centre into a 4:3-safe area even when a UI
triangle batch carries varying Z/Q values. High-Z triangles that write depth,
and lower-Z depth-tested triangles with perspective-varying Z/Q, stay in the
world coordinate system. This protects both ordinary 3D and transparent
projected effects such as car shadows. Other flat masked-depth HUD/menu draws
are corrected as before. The later frontend's lower-Z path uses textured,
blended, depth-tested and depth-writing triangle meshes. Their exactly constant
Z/Q values and useful screen extent identify the screen-space mesh without
capturing the projected car preview drawn in the same frame. Requiring
axis-aligned edges is not safe: curved panels contain non-axis-aligned
triangles, and individual quad halves can differ by a subpixel. Full-surface
sprites and clears always stay on the original path.

Only partial horizontal scissors are transformed. A full-width scissor is
shared by UI, clears, and world geometry and therefore remains full width.
Partial inclusive scissors are converted as half-open intervals with outward
rounding, avoiding dropped edge pixels. Scene detection is used only to select
diagnostic menu/HUD controls and label logs; it does not alter per-draw
classification, so a scene transition cannot reclassify the next frame's
geometry.

## Diagnostics

The three correction controls default to enabled. Set one to `0`, `off`, or
`false` before launch to disable it independently:

- `PS2_2D_MENU=0` disables coordinate correction while the frontend/menu scene
  detector is active.
- `PS2_2D_HUD=0` disables coordinate correction while gameplay is active.
- `PS2_2D_SCISSOR=0` keeps coordinate correction but uses original scissors.

Set both `PS2_2D_MENU=0` and `PS2_2D_HUD=0` to compare with the original GS draw
path while retaining the selected widescreen camera projection. Environment
controls are read once at startup.

`PS2_2D_LOG=1` prints a per-frame classifier summary and frontend/gameplay
transitions. `PS2_2D_LOG=draw` additionally prints every positively matched HUD
draw and at most 64 other suspicious signatures per frame, including classification, texture/blend/depth state,
Z range, frame/depth/texture base pointers, and original/result scissors.

## Manual validation

Run the same race and camera sequence at 4:3, 16:9, 16:10, 21:9, and 32:9.
At each ratio inspect the car shadow while steering, braking, changing camera,
crossing the start line, pausing, and returning to the menu. Check both vertical
safe-area boundaries for single-frame lines or stale pixels. Repeat 16:9 once
with `PS2_2D_SCISSOR=0`, then once with both menu and HUD correction disabled,
and capture `PS2_2D_LOG=draw` if a difference remains.

Automated classification and clipping tests cannot prove the visual result.
21:9 still needs manual culling and camera-clipping coverage; 32:9 remains
experimental.

Keep `PS2_SCALE=2` for the intended Apple Silicon performance profile. The
classifier is deliberately limited to Auto Modellista PAL (SLES-51191).
