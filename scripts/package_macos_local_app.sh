#!/bin/bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd -P)"
BIN="$ROOT/build/native-automodellista-speedometer/ps2run"
ICON="$ROOT/assets/macos/AutoModellistaEnhanced.icns"
GAME="$ROOT/games/automodellista-pal-ws"
TARGET="${AME_PACKAGE_OUTPUT:-$HOME/Applications/Auto Modellista Enhanced (Package Test).app}"

for F in "$BIN" "$ICON" "$GAME/patched/SLES_511.91" "$GAME/disc.iso"; do
  if [[ ! -f "$F" ]]; then
    echo "Missing required file: $F"
    exit 1
  fi
done

if [[ -e "$TARGET" ]]; then
  echo "Test app already exists. Refusing to overwrite:"
  echo "$TARGET"
  exit 1
fi

if [[ " $(lipo -archs "$BIN") " != *" arm64 "* ]]; then
  echo "Game executable is not ARM64."
  exit 1
fi

mkdir -p "$HOME/Applications"

STAGE="$(mktemp -d "$HOME/Applications/.ame-stage.XXXXXX")"
trap 'rm -rf "$STAGE"' EXIT

APP="$STAGE/Auto Modellista Enhanced (Package Test).app"
MACOS="$APP/Contents/MacOS"
LIBS="$APP/Contents/Frameworks"
RES="$APP/Contents/Resources"

mkdir -p "$MACOS" "$LIBS" "$RES"

echo "Copying native game executable and S2000 icon..."

cp -p "$BIN" "$MACOS/ps2run"
cp -p "$ICON" "$RES/AutoModellistaEnhanced.icns"

SDL_DEP="$(
  otool -L "$BIN" |
  awk 'NR > 1 {print $1}' |
  grep 'libSDL2-2\.0\.0\.dylib$' |
  head -n 1 || true
)"

if [[ "$SDL_DEP" != /* || ! -f "$SDL_DEP" ]]; then
  echo "SDL2 library dependency could not be located."
  exit 1
fi

echo "Bundling SDL2..."

cp -L "$SDL_DEP" "$LIBS/libSDL2-2.0.0.dylib"

install_name_tool -change \
  "$SDL_DEP" \
  "@executable_path/../Frameworks/libSDL2-2.0.0.dylib" \
  "$MACOS/ps2run"

install_name_tool -id \
  "@loader_path/libSDL2-2.0.0.dylib" \
  "$LIBS/libSDL2-2.0.0.dylib"

echo "Bundling SDL3 runtime..."

SDL3_SOURCE=""
for CANDIDATE in \
  /opt/homebrew/opt/sdl3/lib/libSDL3.dylib \
  /opt/homebrew/lib/libSDL3.dylib; do
  if [[ -f "$CANDIDATE" ]]; then
    SDL3_SOURCE="$CANDIDATE"
    break
  fi
done

if [[ -z "$SDL3_SOURCE" ]]; then
  echo "SDL3 not found. Install SDL3 before packaging."
  exit 1
fi

if [[ " $(lipo -archs "$SDL3_SOURCE") " != *" arm64 "* ]]; then
  echo "SDL3 does not contain ARM64."
  exit 1
fi

cp -L "$SDL3_SOURCE" "$LIBS/libSDL3.dylib"

SDL3_EXTRA="$(
  otool -L "$LIBS/libSDL3.dylib" |
  awk 'NR > 2 {print $1}' |
  grep -Ev '^(/usr/lib/|/System/Library/)' || true
)"

if [[ -n "$SDL3_EXTRA" ]]; then
  echo "SDL3 has unbundled dependencies:"
  echo "$SDL3_EXTRA"
  exit 1
fi

if otool -l "$MACOS/ps2run" |
   grep -Fq "path /opt/homebrew/lib (offset"; then
  install_name_tool -delete_rpath \
    "/opt/homebrew/lib" "$MACOS/ps2run"
fi

check_deps() {
  local FILE="$1"
  local DEP

  while IFS= read -r DEP; do
    case "$DEP" in
      /usr/lib/*|/System/Library/*)
        ;;
      @executable_path/../Frameworks/libSDL2-2.0.0.dylib)
        ;;
      @loader_path/libSDL2-2.0.0.dylib)
        ;;
      *)
        echo "Unresolved dependency:"
        echo "$FILE -> $DEP"
        return 1
        ;;
    esac
  done < <(otool -L "$FILE" | awk 'NR > 1 {print $1}')
}

check_deps "$MACOS/ps2run"
check_deps "$LIBS/libSDL2-2.0.0.dylib"

echo "Compiling native macOS launcher..."

cat > "$STAGE/launcher.m" <<'OBJC'
#import <Cocoa/Cocoa.h>
#import <unistd.h>
#import <fcntl.h>
#import <errno.h>
#import <string.h>

static void showError(NSString *message) {
    [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    [NSApp activateIgnoringOtherApps:YES];
    NSAlert *alert = [[NSAlert alloc] init];
    alert.messageText = @"Auto Modellista Enhanced";
    alert.informativeText = message;
    [alert runModal];
}

int main(void) {
    @autoreleasepool {
        NSFileManager *fm = [NSFileManager defaultManager];
        NSBundle *bundle = [NSBundle mainBundle];

        NSString *root =
            [bundle objectForInfoDictionaryKey:@"AMEProjectRoot"];
        NSString *game =
            [root stringByAppendingPathComponent:@"games/automodellista-pal-ws"];
        NSString *elf =
            [game stringByAppendingPathComponent:@"patched/SLES_511.91"];
        NSString *disc =
            [game stringByAppendingPathComponent:@"disc.iso"];
        NSString *engine =
            [[bundle bundlePath] stringByAppendingPathComponent:@"Contents/MacOS/ps2run"];
        NSString *support =
            [NSHomeDirectory() stringByAppendingPathComponent:@"Library/Application Support/AutoModellistaEnhanced"];
        NSString *card =
            [support stringByAppendingPathComponent:@"memcard.ps2"];
        NSString *log =
            [support stringByAppendingPathComponent:@"launcher-packaged.log"];

        if (![fm isExecutableFileAtPath:engine] ||
            ![fm fileExistsAtPath:elf] ||
            ![fm fileExistsAtPath:disc]) {
            showError(@"The game executable or PAL game files are missing.");
            return 1;
        }

        NSError *error = nil;
        if (![fm createDirectoryAtPath:support
             withIntermediateDirectories:YES
             attributes:nil error:&error]) {
            showError(error.localizedDescription);
            return 1;
        }

        if (chdir(root.fileSystemRepresentation) != 0) {
            showError(@"Cannot access the game project folder.");
            return 1;
        }

        int fd = open(log.fileSystemRepresentation,
                      O_WRONLY | O_CREAT | O_APPEND, 0600);
        if (fd >= 0) {
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
            close(fd);
        }

        char *args[] = {
            (char *)engine.fileSystemRepresentation,
            (char *)elf.fileSystemRepresentation,
            "--disc", (char *)disc.fileSystemRepresentation,
            "--game-dir", (char *)game.fileSystemRepresentation,
            "--memcard", (char *)card.fileSystemRepresentation,
            "--window",
            "--no-record",
            NULL
        };

        execv(engine.fileSystemRepresentation, args);
        showError([NSString stringWithFormat:
            @"Game launch failed: %s", strerror(errno)]);
        return 1;
    }
}
OBJC

xcrun clang -arch arm64 -fobjc-arc \
  -framework Cocoa \
  "$STAGE/launcher.m" \
  -o "$MACOS/AutoModellistaLauncher"

cat > "$APP/Contents/Info.plist" <<'PLIST'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN"
"http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>CFBundleName</key>
  <string>Auto Modellista Enhanced</string>
  <key>CFBundleDisplayName</key>
  <string>Auto Modellista Enhanced</string>
  <key>CFBundleIdentifier</key>
  <string>com.pharmerdan.automodellistaenhanced.packagetest</string>
  <key>CFBundleExecutable</key>
  <string>AutoModellistaLauncher</string>
  <key>CFBundleIconFile</key>
  <string>AutoModellistaEnhanced.icns</string>
  <key>CFBundlePackageType</key>
  <string>APPL</string>
  <key>CFBundleVersion</key>
  <string>1</string>
  <key>CFBundleShortVersionString</key>
  <string>0.2.1</string>
  <key>NSHighResolutionCapable</key>
  <true/>
</dict>
</plist>
PLIST

/usr/libexec/PlistBuddy -c \
  "Add :AMEProjectRoot string $ROOT" \
  "$APP/Contents/Info.plist"

echo "Validating app bundle..."

if [[ "$(basename "$TARGET")" == "Auto Modellista Enhanced.app" ]]; then
  /usr/libexec/PlistBuddy -c \
    "Set :CFBundleIdentifier com.pharmerdan.automodellistaenhanced.local" \
    "$APP/Contents/Info.plist"
fi

plutil -lint "$APP/Contents/Info.plist"

codesign --force --sign - "$LIBS/libSDL3.dylib"
codesign --force --sign - "$LIBS/libSDL2-2.0.0.dylib"
codesign --force --sign - "$MACOS/ps2run"
codesign --force --sign - "$MACOS/AutoModellistaLauncher"
codesign --force --sign - "$APP"

codesign --verify --deep --strict "$APP"

mv "$APP" "$TARGET"

echo
echo "SUCCESS — test application created:"
echo "$TARGET"
echo
echo "Existing apps, saves and game files unchanged."
