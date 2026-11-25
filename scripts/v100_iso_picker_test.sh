#!/bin/bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd -P)"
REFERENCE="$ROOT/games/automodellista-pal-ws/disc/SLES_511.91"

if [[ ! -f "$REFERENCE" ]]; then
  echo "Known-working PAL reference executable missing."
  exit 1
fi

echo "Opening macOS ISO selection dialog..."

if ! ISO="$(osascript -e \
  'POSIX path of (choose file with prompt "Select your PAL Auto Modellista PS2 ISO (SLES-51191)")')"; then
  echo "File selection cancelled or unavailable."
  exit 0
fi

case "${ISO##*.}" in
  iso|ISO) ;;
  *)
    echo "Please select an ISO file."
    exit 1
    ;;
esac

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

echo
echo "Checking PAL game identification..."

python3 -I "$ROOT/scripts/disc.py" extract \
  "$ISO" "$TMP" SYSTEM.CNF

if [[ ! -f "$TMP/SYSTEM.CNF" ]]; then
  echo "SYSTEM.CNF missing. Unsupported ISO."
  exit 1
fi

if ! grep -Eiq 'SLES_511\.91' "$TMP/SYSTEM.CNF"; then
  echo "Unsupported game or region."
  echo "Expected PAL SLES-51191."
  exit 1
fi

echo "PAL disc identifier recognised."

echo
echo "Checking original game executable..."

python3 -I "$ROOT/scripts/disc.py" extract \
  "$ISO" "$TMP" SLES_511.91

if [[ ! -f "$TMP/SLES_511.91" ]]; then
  echo "Expected game executable missing."
  exit 1
fi

if ! cmp -s "$TMP/SLES_511.91" "$REFERENCE"; then
  echo "PAL game executable differs from the verified version."
  echo "Further validation is required before import."
  exit 1
fi

echo
echo "====================================="
echo " SUCCESS — Supported PAL ISO"
echo "====================================="
echo
echo "Game: Auto Modellista"
echo "Region: PAL"
echo "Disc: SLES-51191"
echo "Executable: Exact match"
echo
echo "Verified original executable SHA-256:"
shasum -a 256 "$TMP/SLES_511.91"

echo
echo "Ready for local game preparation."
echo "No compilation performed and no game files modified."
