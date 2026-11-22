# Auto Modellista Enhanced — Phase 2C

Phase 2C adds independent CRT presentation and opt-in race-HUD edge anchoring
for PAL `SLES-51191`. Both default to Off/original and persist in the existing
enhanced settings file.

## CRT presentation

The existing WebGPU blit pass now contains Off, CRT-Lottes Fast, CRT-Lottes and
CRT-Hyllian branches. It reads the GS presentation buffer directly and does not
allocate an intermediate full-screen texture. Scanline intensity, mask strength,
curvature, brightness and sharpness are uniform parameters and update live.
The host overlay is still drawn later in the same render pass, so F10, F1 and F3
remain sharp. Off takes the original shader branch unchanged.

The WGSL implementations are portable, single-pass translations of the public
Lottes algorithm and the established Hyllian reconstruction/scanline design;
no Xbox renderer code or vendored RetroArch shader source was copied. The
reference project's MIT-licensed architecture and documented parameter choices
were used as design guidance. Runtime visual equivalence to its librashader
presets still requires manual comparison on an unrestricted Metal desktop.

## PAL race-HUD identification

A saved race at frame 4580 and its GS draw stream identified these groups:

- left: position and three time rows, original bounds x=22..151, y=25..202;
- right: lap counter and course map, original bounds x=486..616, y=19..181;
- unchanged: speedometer/tachometer (below y=240), centred race messages and
  the centred pause/menu paths.

The classifier requires the exact PAL sprite path, high interface layer, the
observed HUD/map texture pages and one of the two measured bounds. It does not
use primitive type, depth or Z as a broad relocation heuristic. Geometry is
first aspect-corrected as before, then translated by the 4:3-safe inset minus
the selected margin. Its size and aspect are unchanged. Partial scissors are
translated and clipped; the shared full-frame scissor remains untouched.

The restored-state capture used texture pages `37a0/38a0/3aa0`; a normal boot
uses the corresponding `2b00/2c00/2e00` pages. At 4:3 the translation is zero.
These are raw `TEX0.TBP0` field values, matching the `tbp` values emitted by
`PS2_2D_LOG=draw`; they are not byte addresses.

## Manual validation

At 2x, test Off and all three CRT modes, adjust every slider live, and verify
F10/F1/F3 stay sharp. In a race, switch HUD positioning live at 4:3, 16:9,
16:10, 21:9 and 32:9; inspect position, timing rows, lap/map, speedometer,
centred messages and pause. Repeat while steering/braking and changing camera,
checking car shadows and every screen boundary. Compare presented FPS and GPU
time against Off, especially CRT-Lottes and 32:9. Visual/gameplay success is not
claimed until this checklist is completed on an interactive Metal desktop.

For focused HUD diagnostics, launch with `PS2_2D_LOG=draw`. Matched race draws
report `hud left` or `hud right`, the live `edges` and `margin` settings, the
signed `shift`, and the final two `outx` coordinates. At 16:10 with a 16-pixel
margin, matches should report `shift -37` for the left group and `shift +37`
for the right group; switching to Original retains the match but reports
`edges 0` and `shift +0`.
