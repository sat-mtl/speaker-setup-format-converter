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
cmake --install "$BUILD_DIR" --component "$COMPONENT" --prefix "$STAGE/$TOPDIR" >/dev/null

# Flatten: a zip the user unpacks and double-clicks should not bury the .exe under bin/.
mv "$STAGE/$TOPDIR/bin/$SLUG.exe" "$STAGE/$TOPDIR/"
mv "$STAGE/$TOPDIR/bin/spatparsecli.exe" "$STAGE/$TOPDIR/"
rmdir "$STAGE/$TOPDIR/bin"
mv "$STAGE/$TOPDIR/share/doc/$SLUG/README.md" "$STAGE/$TOPDIR/"
rm -rf "$STAGE/$TOPDIR/share"

step "dependency closure"
# Qt is linked statically, but the SDK's clang still links libc++ and libunwind as DLLs,
# so the zip has to carry them -- without them the .exe does not start on a machine that
# has no ossia SDK, which is every machine we ship to.
#
# "Is this a system DLL" is decided by looking in %WINDIR%\System32 rather than by matching
# names, so a dependency that appears later cannot pass the check by looking plausible.
# Anything else is resolved against the toolchain's own bin directories and copied in.
OBJDUMP="${OBJDUMP:-}"
if [[ -z "$OBJDUMP" ]]; then
  OBJDUMP=$(command -v llvm-objdump || command -v objdump || true)
fi
[[ -n "$OBJDUMP" ]] || die "no llvm-objdump/objdump on PATH: cannot verify the import table"

SYS32="${SYS32:-/c/Windows/System32}"
# Where a non-system DLL may legitimately come from. CMAKE_CXX_COMPILER's directory is the
# SDK's llvm/bin, which is where libc++.dll and libunwind.dll live.
cxx=$(sed -n 's/^CMAKE_CXX_COMPILER:[^=]*=//p' "$BUILD_DIR/CMakeCache.txt" | head -1)
SEARCH_DIRS=("$(dirname "$(cygpath -u "$cxx" 2>/dev/null || echo "$cxx")")")

imports() { "$OBJDUMP" -p "$1" | sed -n 's/^[[:space:]]*DLL Name: *//p' | sort -u; }

queue=("$STAGE/$TOPDIR/$SLUG.exe" "$STAGE/$TOPDIR/spatparsecli.exe")
seen=" "
missing=""
bundled=""
while [[ ${#queue[@]} -gt 0 ]]; do
  bin="${queue[0]}"; queue=("${queue[@]:1}")
  echo "   $(basename "$bin"):"
  while IFS= read -r dll; do
    [[ -n "$dll" ]] || continue
    lower=$(echo "$dll" | tr 'A-Z' 'a-z')
    if [[ -f "$STAGE/$TOPDIR/$dll" ]]; then
      echo "     $dll [bundled]"
    elif compgen -G "$SYS32/$dll" >/dev/null 2>&1 || compgen -G "$SYS32/$lower" >/dev/null 2>&1; then
      echo "     $dll"
    else
      found=""
      for d in "${SEARCH_DIRS[@]}"; do
        [[ -f "$d/$dll" ]] && { found="$d/$dll"; break; }
      done
      if [[ -n "$found" ]]; then
        cp "$found" "$STAGE/$TOPDIR/$dll"
        bundled="$bundled $dll"
        echo "     $dll -> bundled from $(dirname "$found")"
        # Its own imports have to be satisfied too.
        [[ "$seen" == *" $lower "* ]] || { seen="$seen$lower "; queue+=("$STAGE/$TOPDIR/$dll"); }
      else
        echo "     $dll [MISSING]"
        missing="$missing $dll"
      fi
    fi
  done < <(imports "$bin")
done
[[ -z "$missing" ]] || die "imports DLLs that are neither in System32 nor in the toolchain:$missing"
echo "   bundled runtime:${bundled:- none}"

step "zip"
# cmake -E tar, not zip/7z: cmake is the one tool guaranteed present on a build host.
rm -f "$OUTPUT"
(cd "$STAGE" && cmake -E tar cf "$OUTPUT" --format=zip "$TOPDIR")

step "result"
ls -l "$OUTPUT"
cmake -E sha256sum "$OUTPUT"
