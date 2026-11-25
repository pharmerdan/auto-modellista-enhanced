## Auto Modellista Enhanced — v1.0.0-beta.1

First public Apple Silicon beta of Auto Modellista Enhanced.

### Features

- Native ARM64 recompilation of the PAL PS2 version
- First-run ISO selection and validation
- Automatic local game preparation
- Native macOS launcher with preparation progress bar
- Persistent prepared game installation
- Enhanced graphics options and widescreen support
- Controller support and virtual memory card saves

### Requirements

- Apple Silicon Mac
- Your own supported PAL Auto Modellista ISO (SLES-51191)
- Xcode Command Line Tools
- Homebrew dependencies: `bash`, `cmake`, `ninja`, `sdl2-compat`, `sdl3`

### Installation

Download the DMG, read **READ ME FIRST.txt**, and drag the app to Applications.

This beta is **not notarized by Apple**. macOS may require manual approval through **System Settings → Privacy & Security → Open Anyway**.

### Important limitations

- First-run recompilation requires external development tools.
- Clean-Mac installation compatibility is still being evaluated.
- This beta supports the verified PAL game revision.
- No copyrighted game ISO or extracted game executable is distributed.

### Credits

Based on ps2-native-recomp by exoteric33 and ps2-web-recomp by Samir Nuri.

PAL widescreen patch by nemesis2000, redistributed with permission confirmed by the project maintainer.

Auto Modellista is the property of its respective rights holders. This is an unofficial project.
