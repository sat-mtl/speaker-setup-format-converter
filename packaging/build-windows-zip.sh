#!/usr/bin/env bash
# Package the Windows build as a zip -- the delivery format the suite's Windows artifacts
# use (there is no installer for the tools, only score itself ships one).
#
# Run from Git Bash; the SDK's clang is what builds this project on Windows anyway, so a
# POSIX shell is already a prerequisite.
#
#   cmake -S . -B build ... && cmake --build build
#   packaging/build-windows-zip.sh
set -euo pipefail
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

ARCH=$(host_arch_tag)
STAGE="${STAGE:-$BUILD_DIR/zipstage}"
TOPDIR="$SLUG-$VERSION"
OUTPUT="$OUT_DIR/$SLUG-$VERSION-windows-$ARCH.zip"

require_build
mkdir -p "$OUT_DIR"

step "install tree -> $STAGE/$TOPDIR"
rm -rf "$STAGE"
mkdir -p "$STAGE/$TOPDIR"
cmake --install "$BUILD_DIR" --prefix "$STAGE/$TOPDIR" >/dev/null

# Flatten: a zip the user unpacks and double-clicks should not bury the .exe under bin/.
mv "$STAGE/$TOPDIR/bin/$SLUG.exe" "$STAGE/$TOPDIR/"
mv "$STAGE/$TOPDIR/bin/spatparsecli.exe" "$STAGE/$TOPDIR/"
rmdir "$STAGE/$TOPDIR/bin"
mv "$STAGE/$TOPDIR/share/doc/$SLUG/README.md" "$STAGE/$TOPDIR/"
rm -rf "$STAGE/$TOPDIR/share"

step "dependency check"
# The SDK build links Qt statically, so the only imports should be system DLLs from
# %WINDIR%. A Qt or libc++ DLL in this list means the zip is missing a file and would
# fail to start on a clean machine.
OBJDUMP="${OBJDUMP:-}"
if [[ -z "$OBJDUMP" ]]; then
  OBJDUMP=$(command -v llvm-objdump || command -v objdump || true)
fi
if [[ -n "$OBJDUMP" ]]; then
  nonsystem=""
  for exe in "$STAGE/$TOPDIR/$SLUG.exe" "$STAGE/$TOPDIR/spatparsecli.exe"; do
    echo "   $(basename "$exe"):"
    dlls=$("$OBJDUMP" -p "$exe" | sed -n 's/^[[:space:]]*DLL Name: *//p' | sort -u)
    echo "$dlls" | sed 's/^/     /'
    hits=$(echo "$dlls" | grep -iE '^(Qt6|libc\+\+|libunwind|zlib|brotli|freetype|harfbuzz)' || true)
    [[ -z "$hits" ]] || nonsystem="$nonsystem$exe: $hits
"
  done
  [[ -z "$nonsystem" ]] || die "imports libraries that the zip does not ship:
$nonsystem"
else
  echo "   no objdump available: import table NOT checked"
fi

step "zip"
# cmake -E tar, not zip/7z: cmake is the one tool guaranteed present on a build host.
rm -f "$OUTPUT"
(cd "$STAGE" && cmake -E tar cf "$OUTPUT" --format=zip "$TOPDIR")

step "result"
ls -l "$OUTPUT"
cmake -E sha256sum "$OUTPUT"
