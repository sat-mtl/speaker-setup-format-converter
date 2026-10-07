# Shared by the three packaging scripts. Sourced, not executed.
#
# Artifact names follow the convention sat-toolbox's download_artifacts table enforces:
#   <slug>-<version>-<platform>-<arch>.<ext>
# with platform-arch drawn from the set in
# supabase/migrations/20260729000001_multiarch_platforms.sql.

SLUG=speaker-layout-converter
APP_DISPLAY_NAME="Speaker Layout Converter"
BUNDLE_ID=ca.qc.sat.speaker-layout-converter

PKG_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$PKG_ROOT/build}"
OUT_DIR="${OUT_DIR:-$PKG_ROOT/packages}"

# The store keys artifacts on major.minor; the full project version goes in the bundle
# metadata instead.
if [[ -z "${VERSION:-}" ]]; then
  _full=$(sed -n 's/^[[:space:]]*VERSION[[:space:]]\+\([0-9][0-9.]*\).*/\1/p' \
          "$PKG_ROOT/CMakeLists.txt" | head -1)
  [[ -n "$_full" ]] || { echo "cannot read VERSION from CMakeLists.txt" >&2; exit 1; }
  VERSION="${_full%.*}"
fi

die() { echo "ERROR: $*" >&2; exit 1; }
step() { printf '\n== %s\n' "$*"; }

host_arch_tag() {
  case "$(uname -m)" in
    x86_64|amd64)  echo x86_64 ;;
    arm64|aarch64) echo arm64 ;;
    *) die "unsupported architecture $(uname -m)" ;;
  esac
}

# Fails loudly rather than packaging a tree that was never built.
require_build() {
  [[ -d "$BUILD_DIR" ]] || die "no build directory at $BUILD_DIR (set BUILD_DIR=)"
  [[ -f "$BUILD_DIR/CMakeCache.txt" ]] || die "$BUILD_DIR is not a CMake build directory"
}
