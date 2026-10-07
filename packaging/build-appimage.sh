#!/usr/bin/env bash
# Package the Linux build as an AppImage -- the delivery format every other Linux GUI tool
# in the suite ships (livepose, koaia, Domeport Pro, spatial-protocol-mapper).
#
#   cmake -S . -B build ... && cmake --build build
#   packaging/build-appimage.sh
#
# BUILD_DIR, OUT_DIR, VERSION and APPIMAGETOOL can all be overridden from the environment.
set -euo pipefail
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

ARCH=$(host_arch_tag)
APPDIR="${APPDIR:-$BUILD_DIR/AppDir}"
OUTPUT="$OUT_DIR/$SLUG-$VERSION-linux-$ARCH.AppImage"

require_build
mkdir -p "$OUT_DIR"

step "install tree -> $APPDIR"
rm -rf "$APPDIR"
cmake --install "$BUILD_DIR" --component "$COMPONENT" --prefix "$APPDIR/usr" >/dev/null
[[ -x "$APPDIR/usr/bin/$SLUG" ]] || die "$SLUG missing from the install tree"
[[ -x "$APPDIR/usr/bin/spatparsecli" ]] || die "spatparsecli missing from the install tree"

# appimagetool reads these from the AppDir root, not from usr/share.
cp "$APPDIR/usr/share/applications/$SLUG.desktop" "$APPDIR/$SLUG.desktop"
cp "$APPDIR/usr/share/icons/hicolor/256x256/apps/$SLUG.png" "$APPDIR/$SLUG.png"
ln -sf "$SLUG.png" "$APPDIR/.DirIcon"

# One AppImage carries both halves of the tool: the GUI by default, the CLI when the
# image is invoked as `spatparsecli` (symlink or argv[0]) or with --cli first.
cat > "$APPDIR/AppRun" <<'APPRUN'
#!/bin/sh
HERE="$(dirname "$(readlink -f "$0")")"
export PATH="$HERE/usr/bin:$PATH"

case "${1:-}" in
  --cli) shift; exec "$HERE/usr/bin/spatparsecli" "$@" ;;
esac

case "$(basename "${ARGV0:-$0}")" in
  spatparsecli*) exec "$HERE/usr/bin/spatparsecli" "$@" ;;
esac

exec "$HERE/usr/bin/speaker-layout-converter" "$@"
APPRUN
chmod +x "$APPDIR/AppRun"

step "dependency check"
# The SDK build links Qt statically; anything outside the glibc/X11 baseline would make
# the AppImage non-portable, which is exactly the class of defect that shipped on the
# other two platforms.
for bin in "$APPDIR/usr/bin/$SLUG" "$APPDIR/usr/bin/spatparsecli"; do
  echo "   $(basename "$bin"):"
  ldd "$bin" | awk '{print "     "$1}' | sort
done
nonsystem=$(ldd "$APPDIR/usr/bin/$SLUG" "$APPDIR/usr/bin/spatparsecli" \
            | sed -n 's/.*=> *\([^ ]*\).*/\1/p' | grep -E '^(/usr/local|/opt)/' || true)
[[ -z "$nonsystem" ]] || die "links libraries outside the system prefix:
$nonsystem"

step "appimagetool"
TOOL="${APPIMAGETOOL:-}"
if [[ -z "$TOOL" ]]; then
  TOOL=$(command -v appimagetool || true)
fi
if [[ -z "$TOOL" ]]; then
  TOOL="$BUILD_DIR/appimagetool-$(uname -m).AppImage"
  if [[ ! -x "$TOOL" ]]; then
    echo "   downloading appimagetool"
    curl -sSL -o "$TOOL" \
      "https://github.com/AppImage/appimagetool/releases/download/continuous/appimagetool-$(uname -m).AppImage"
    chmod +x "$TOOL"
  fi
fi
echo "   using $TOOL"

# No FUSE in the almalinux CI container, and none in most build sandboxes.
export APPIMAGE_EXTRACT_AND_RUN=1
export ARCH
rm -f "$OUTPUT"
"$TOOL" --no-appstream "$APPDIR" "$OUTPUT"

step "result"
ls -l "$OUTPUT"
sha256sum "$OUTPUT"
