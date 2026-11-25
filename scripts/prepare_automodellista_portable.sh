#!/bin/bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd -P)"

if [[ $# -ne 2 ]]; then
  echo "Usage: $0 /path/to/game.iso build-name"
  exit 1
fi

NAME="$2"

case "$NAME" in
  ""|*[!a-zA-Z0-9_-]*)
    echo "Invalid build name."
    exit 1
    ;;
esac

"$ROOT/scripts/prepare_automodellista_v100.sh" "$1" "$NAME"

BIN="$ROOT/build/native-$NAME/ps2run"
BIN_DIR="$(dirname "$BIN")"

if [[ ! -x "$BIN" ]]; then
  echo "Prepared executable missing."
  exit 1
fi

for LIB in libSDL2-2.0.0.dylib libSDL3.dylib; do
  if [[ ! -f "$ROOT/runtime/$LIB" ]]; then
    echo "Missing packaged SDL runtime: $LIB"
    exit 1
  fi

  cp -p "$ROOT/runtime/$LIB" "$BIN_DIR/$LIB"
done

SDL_LINK="$(
  otool -L "$BIN" |
  awk '$1 ~ /libSDL2-2\.0\.0\.dylib$/ {print $1; exit}'
)"

if [[ -z "$SDL_LINK" ]]; then
  echo "Cannot identify the SDL2 runtime dependency."
  exit 1
fi

if [[ "$SDL_LINK" != "@executable_path/libSDL2-2.0.0.dylib" ]]; then
  install_name_tool \
    -change "$SDL_LINK" \
    "@executable_path/libSDL2-2.0.0.dylib" \
    "$BIN"
fi

codesign --force --sign - "$BIN"
codesign --verify --strict "$BIN"

echo
echo "PORTABLE GAME READY"
echo "Native executable: $BIN"
echo "SDL runtime libraries: $BIN_DIR"
