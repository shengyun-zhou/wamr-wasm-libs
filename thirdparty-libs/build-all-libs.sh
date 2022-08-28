#!/bin/bash
set -e
cd "$(dirname "$0")"
source config.sh

./build-libiconv.sh
./build-bzip2.sh
./build-zlib.sh
./build-lzma.sh
./build-zstd.sh
./build-brotli.sh
./build-libxml2.sh
./build-libpng.sh
./build-openssl.sh
./build-nghttp2.sh
./build-cares.sh
./build-curl.sh
./build-libmicrohttpd.sh

# Clean up
rm -rf "$OUTPUT_SYSROOT/$SYSROOT_LIBDIR_PREFIX/pkgconfig" || true
rm -rf "$OUTPUT_SYSROOT/$SYSROOT_LIBDIR_PREFIX/cmake" || true
rm -f "$OUTPUT_SYSROOT/$SYSROOT_LIBDIR_PREFIX/"*.la || true
rm -rf "$OUTPUT_SYSROOT/bin" || true
