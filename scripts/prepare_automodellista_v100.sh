#!/bin/bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd -P)"
PATCH="$ROOT/local-patches/SLES-51191_692C87AD.pnach"
ISO="${1:-}"
NAME="${2:-automodellista-v100-import-test}"

if [[ -z "$ISO" ]]; then
  echo "Usage:"
  echo "  $0 /path/to/AutoModellista.iso [build-name]"
  exit 1
fi

case "$NAME" in
  ""|*[!a-zA-Z0-9_-]*)
    echo "Invalid build name: $NAME"
    exit 1
    ;;
esac

if ! command -v brew >/dev/null 2>&1; then
  echo "Homebrew is required for this development-stage preparation."
  exit 1
fi

BREW_BASH="$(brew --prefix)/bin/bash"

if [[ ! -x "$BREW_BASH" ]]; then
  echo "Homebrew Bash is missing."
  echo "Install it with: brew install bash"
  exit 1
fi

if [[ ! -f "$ISO" ]]; then
  echo "ISO not found: $ISO"
  exit 1
fi

if [[ ! -f "$PATCH" ]]; then
  echo "Widescreen patch not found: $PATCH"
  exit 1
fi

ISO="$(python3 -c 'import os,sys; print(os.path.realpath(sys.argv[1]))' "$ISO")"

for DIR in \
  "$ROOT/games/$NAME" \
  "$ROOT/generated/$NAME" \
  "$ROOT/build/native-$NAME"; do
  if [[ -e "$DIR" ]]; then
    echo "Existing output detected: $DIR"
    echo "Preparation cancelled to avoid overwriting files."
    exit 1
  fi
done

cd "$ROOT"

echo "=========================================="
echo " Auto Modellista Enhanced v1.00"
echo " Local PAL game preparation"
echo "=========================================="
echo
echo "ISO: $ISO"
echo "Build: $NAME"
echo
echo "Extracting, patching and recompiling..."

PS2_JOBS="${PS2_JOBS:-4}" "$BREW_BASH" scripts/port.sh \
  "$NAME" \
  "$ISO" \
  --native-only \
  --pnach "$PATCH" \
  --pnach-section "Widescreen 16:9"

BIN="$ROOT/build/native-$NAME/ps2run"
ELF="$ROOT/games/$NAME/patched/SLES_511.91"
DISC="$ROOT/games/$NAME/disc.iso"
PATCHES="$ROOT/games/$NAME/patches.txt"

for FILE in "$BIN" "$ELF" "$DISC" "$PATCHES"; do
  if [[ ! -f "$FILE" ]]; then
    echo "Preparation incomplete. Missing: $FILE"
    exit 1
  fi
done

if [[ " $(lipo -archs "$BIN") " != *" arm64 "* ]]; then
  echo "Prepared executable is not ARM64."
  exit 1
fi

echo
echo "=========================================="
echo " SUCCESS — Native game prepared"
echo "=========================================="
echo
echo "Executable: $BIN"
echo "Game files: $ROOT/games/$NAME"
echo
echo "The original ISO and existing saves were not modified."
