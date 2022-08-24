#!/bin/bash
set -e
cd "$(dirname "$0")"
source config.sh

./build-bzip2.sh
./build-zlib.sh

# Clean up
rm -rf "$OUTPUT_SYSROOT/$SYSROOT_LIBDIR_PREFIX/pkgconfig" || true
rm -f "$OUTPUT_SYSROOT/$SYSROOT_LIBDIR_PREFIX/"*.la || true
