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
export LD_LIBRARY_PATH="$HERE/usr/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

case "${1:-}" in
  --cli) shift; exec "$HERE/usr/bin/spatparsecli" "$@" ;;
esac

case "$(basename "${ARGV0:-$0}")" in
  spatparsecli*) exec "$HERE/usr/bin/spatparsecli" "$@" ;;
esac

exec "$HERE/usr/bin/speaker-layout-converter" "$@"
APPRUN
chmod +x "$APPDIR/AppRun"

BINS=("$APPDIR/usr/bin/$SLUG" "$APPDIR/usr/bin/spatparsecli")

step "bundle the xcb helpers"
# Qt's xcb platform plugin needs these six, and they are the ones a target distro is most
# likely to be missing or to carry at a different soname -- so they are exactly the set the
# suite's other AppImages ship (checked against ossia score's: libxcb-util, -render-util,
# -keysyms, -image, -icccm, -cursor and nothing else). libX11, the core libxcb, libGL and
# friends are deliberately NOT bundled: they have to match the host's display stack and
# graphics driver, and overriding them is how an AppImage breaks on the machines it was
# meant to run on.
mkdir -p "$APPDIR/usr/lib"
for soname in libxcb-util.so.1 libxcb-render-util.so.0 libxcb-keysyms.so.1 \
              libxcb-image.so.0 libxcb-icccm.so.4 libxcb-cursor.so.0; do
  src=$(ldd "${BINS[@]}" | sed -n "s|^[[:space:]]*$soname => \\([^ ]*\\).*|\\1|p" | head -1)
  if [[ -n "$src" && -f "$src" ]]; then
    cp -L "$src" "$APPDIR/usr/lib/$soname"
    echo "   $soname <- $src"
  else
    echo "   $soname: not linked, skipped"
  fi
done

step "dependency check"
# The SDK build links Qt statically; anything outside the glibc/X11 baseline would make
# the AppImage non-portable, which is exactly the class of defect that shipped on the
# other two platforms.
for bin in "${BINS[@]}"; do
  echo "   $(basename "$bin"):"
  ldd "$bin" | awk '{print "     "$1}' | sort
done
nonsystem=$(ldd "${BINS[@]}" \
            | sed -n 's/.*=> *\([^ ]*\).*/\1/p' | grep -E '^(/usr/local|/opt)/' || true)
[[ -z "$nonsystem" ]] || die "links libraries outside the system prefix:
$nonsystem"

step "glibc baseline"
# The guard above only looks at *where* a library came from, so it happily passed an
# AppImage built on a rolling-release host that required GLIBC_2.43 and could not start on
# anything older -- including the almalinux 9 container CI builds in. glibc symbols are
# versioned, so the highest one referenced is the real floor.
BASELINE="${GLIBC_BASELINE:-2.34}"   # almalinux 9, which is what the CI job uses
worst=0
for bin in "${BINS[@]}"; do
  need=$(objdump -T "$bin" 2>/dev/null | grep -oE 'GLIBC_[0-9]+\.[0-9]+' \
         | sed 's/GLIBC_//' | sort -V | tail -1)
  echo "   $(basename "$bin"): needs glibc ${need:-none}"
  [[ -n "$need" ]] || continue
  if [[ "$(printf '%s\n%s\n' "$BASELINE" "$need" | sort -V | tail -1)" != "$BASELINE" ]]; then
    worst=1
  fi
done
[[ "$worst" == 0 ]] || die "built against a glibc newer than the $BASELINE baseline, so this
AppImage will not start on the distributions we target. Build it the way CI does:
  docker run --rm -v \"\$PWD\":/src -w /src almalinux:9 ...
or set GLIBC_BASELINE= to accept a higher floor deliberately."

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
