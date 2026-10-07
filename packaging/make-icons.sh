#!/usr/bin/env bash
# Regenerate the committed icon rasters from speaker-layout-converter.svg.
# The rasters are committed so that packaging needs neither ImageMagick nor Python;
# run this only when the SVG changes.
#
# ImageMagick's own ICNS writer emits a single bare PNG frame, which Finder and
# iconutil both reject, so the .icns container is assembled here instead.
set -euo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")"
SVG=speaker-layout-converter.svg
BASE=speaker-layout-converter

command -v magick >/dev/null || { echo "ImageMagick (magick) is required"; exit 1; }
command -v python3 >/dev/null || { echo "python3 is required"; exit 1; }

tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT

for s in 16 32 48 64 128 256 512 1024; do
  magick -background none "$SVG" -resize "${s}x${s}" -depth 8 "$tmp/$s.png"
done

cp "$tmp/256.png" "$BASE.png"

magick "$tmp/256.png" -define icon:auto-resize=256,128,64,48,32,16 "$BASE.ico"

# icns: 'icns' + total length, then one typed chunk per representation.
python3 - "$tmp" "$BASE.icns" <<'PY'
import struct, sys

tmp, out = sys.argv[1], sys.argv[2]
# OSType -> pixel size, per Apple's icon type table.
reps = [("icp4", 16), ("icp5", 32), ("ic07", 128), ("ic08", 256),
        ("ic09", 512), ("ic10", 1024), ("ic11", 32), ("ic12", 64),
        ("ic13", 256), ("ic14", 512)]

body = b""
for ostype, size in reps:
    with open(f"{tmp}/{size}.png", "rb") as f:
        png = f.read()
    body += ostype.encode("ascii") + struct.pack(">I", len(png) + 8) + png

with open(out, "wb") as f:
    f.write(b"icns" + struct.pack(">I", len(body) + 8) + body)
PY

ls -l "$BASE.png" "$BASE.ico" "$BASE.icns"
