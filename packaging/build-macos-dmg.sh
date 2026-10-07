#!/usr/bin/env bash
# Package the macOS build as a signed .app inside a .dmg -- the delivery format the rest of
# the suite ships (ossia score, Domeport Pro, the create-app-macos.sh tools).
#
#   cmake -S . -B build ... && cmake --build build
#   packaging/build-macos-dmg.sh
#
# Signing happens when MAC_CODESIGN_IDENTITY is set, notarization additionally when
# NOTARY_PROFILE is set. Both are skipped with a loud warning otherwise, so an unsigned
# .dmg is never mistaken for a signed one.
set -euo pipefail
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

[[ "$(uname -s)" == Darwin ]] || die "this script only runs on macOS"

ARCH=$(host_arch_tag)
APP_NAME="$APP_DISPLAY_NAME.app"
STAGE="${STAGE:-$BUILD_DIR/dmgstage}"
APP="$STAGE/$APP_NAME"
OUTPUT="$OUT_DIR/$SLUG-$VERSION-macos-$ARCH.dmg"
ENTITLEMENTS="$PKG_ROOT/packaging/entitlements.plist"

IDENTITY="${MAC_CODESIGN_IDENTITY:-}"
PROFILE="${NOTARY_PROFILE:-}"
# notarytool reads the profile from the keychain that stores it, which on this fleet is the
# login keychain rather than the (possibly different) default one.
NOTARY_KEYCHAIN="${NOTARY_KEYCHAIN:-$HOME/Library/Keychains/login.keychain-db}"

require_build
mkdir -p "$OUT_DIR"

step "install tree -> $STAGE"
rm -rf "$STAGE"
mkdir -p "$STAGE"
cmake --install "$BUILD_DIR" --component "$COMPONENT" --prefix "$STAGE" >/dev/null
[[ -d "$APP" ]] || die "$APP_NAME missing from the install tree"

# One artifact carries both halves of the tool. Apple's layout for a helper executable that
# is not itself a bundle is Contents/MacOS, next to the GUI binary; `install` put the CLI in
# bin/ because that is the right place on Linux and Windows.
mv "$STAGE/bin/spatparsecli" "$APP/Contents/MacOS/spatparsecli"
rmdir "$STAGE/bin"
mv "$STAGE/share/doc/$SLUG/README.md" "$STAGE/README.md"
rm -rf "$STAGE/share"

step "dependency closure"
# The SDK build links Qt statically. A load command pointing into /opt/homebrew or
# /usr/local would make the .app launch only on a machine that happens to have the same
# Homebrew formula installed -- the defect class this packaging exists to close.
mach_o() { file -b "$1" | grep -q 'Mach-O'; }

BINARIES=()
while IFS= read -r f; do
  mach_o "$f" && BINARIES+=("$f")
done < <(find "$APP" -type f -perm -u+x)
[[ ${#BINARIES[@]} -gt 0 ]] || die "no Mach-O files found in $APP"

bad=""
for b in "${BINARIES[@]}"; do
  echo "   ${b#$APP/}"
  otool -L "$b" | tail -n +2 | awk '{print "     "$1}'
  hits=$(otool -L "$b" | tail -n +2 | awk '{print $1}' \
         | grep -E '^(/opt/homebrew|/usr/local)' || true)
  [[ -z "$hits" ]] || bad="$bad$b:
$hits
"
done
[[ -z "$bad" ]] || die "load commands resolve outside the system prefix:
$bad"

step "deployment target"
# Same class of defect as the load commands above, and just as invisible on the machine that
# built it: with CMAKE_OSX_DEPLOYMENT_TARGET unset the binaries are stamped with the build
# host's OS version and refuse to launch on anything older. Checked against what CMake was
# actually told rather than a constant, so the two cannot drift apart.
want=$(sed -n 's/^CMAKE_OSX_DEPLOYMENT_TARGET:[^=]*=//p' "$BUILD_DIR/CMakeCache.txt" | head -1)
[[ -n "$want" ]] || die "CMAKE_OSX_DEPLOYMENT_TARGET is not set in $BUILD_DIR"
for b in "${BINARIES[@]}"; do
  got=$(otool -l "$b" | awk '/LC_BUILD_VERSION/,/^$/' | awk '$1=="minos"{print $2; exit}')
  echo "   ${b#$APP/}: minos $got"
  [[ "$got" == "$want" || "$got" == "$want."* ]] \
    || die "${b#$APP/} is built for macOS $got, not the requested $want"
done

if [[ -n "$IDENTITY" ]]; then
  step "codesign"
  # Inside out: the bundle seal covers the nested binaries, so re-signing a nested binary
  # afterwards would invalidate it. Deepest path first gives that order without special
  # cases. Every Mach-O is signed, not just dylibs -- an unsigned helper executable is
  # what makes --deep --strict fail after the fact.
  while IFS= read -r b; do
    codesign --force --timestamp --options=runtime \
      --sign "$IDENTITY" "$b"
  done < <(printf '%s\n' "${BINARIES[@]}" | awk '{print gsub(/\//,"/")" "$0}' \
           | sort -rn | cut -d' ' -f2-)

  codesign --entitlements "$ENTITLEMENTS" --force --timestamp \
    --options=runtime --sign "$IDENTITY" "$APP"

  step "codesign --verify --deep --strict"
  codesign --verify --deep --strict --verbose=2 "$APP"
  step "spctl assessment"
  # Unnotarized Developer ID code is rejected here by design; the signature itself is what
  # --verify above proves. Report the verdict either way rather than failing the build.
  spctl -a -t exec -vv "$APP" || true
else
  step "codesign SKIPPED"
  echo "   MAC_CODESIGN_IDENTITY is unset: the .app and .dmg are UNSIGNED."
fi

step "dmg"
# hdiutil, not create-dmg: create-dmg's window-prettifying step drives Finder over
# AppleScript and its retries need sudo, neither of which exists on a headless builder.
rm -f "$OUTPUT"
hdiutil create -quiet -format UDZO -srcfolder "$STAGE" \
  -volname "$APP_DISPLAY_NAME $VERSION" "$OUTPUT"

if [[ -n "$IDENTITY" ]]; then
  codesign --force --timestamp --sign "$IDENTITY" "$OUTPUT"
  codesign --verify --strict --verbose=2 "$OUTPUT"
fi

if [[ -n "$IDENTITY" && -n "$PROFILE" ]]; then
  step "notarize"
  xcrun notarytool submit "$OUTPUT" \
    --keychain-profile "$PROFILE" --keychain "$NOTARY_KEYCHAIN" --wait
  xcrun stapler staple "$OUTPUT"
  xcrun stapler validate "$OUTPUT"
  step "spctl assessment, stapled"
  spctl -a -t open --context context:primary-signature -vv "$OUTPUT"
else
  step "notarization SKIPPED"
  echo "   NOTARY_PROFILE is unset: the .dmg is NOT notarized and Gatekeeper will"
  echo "   refuse it on a machine that did not build it."
fi

step "result"
ls -l "$OUTPUT"
shasum -a 256 "$OUTPUT"
