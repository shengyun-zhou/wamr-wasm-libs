#!/bin/bash
set -e
cd "$(dirname "$0")"
source config.sh

SOURCE_TARBALL=brotli-${BROTLI_VERSION}.tar.gz
mkdir -p "$SOURCE_DIR"
if [ ! -f "$SOURCE_DIR/$SOURCE_TARBALL" ]; then
    curl -sSL "https://${GITHUB_MIRROR_DOMAIN:-github.com}/google/brotli/archive/refs/tags/v${BROTLI_VERSION}.tar.gz" -o "$SOURCE_DIR/$SOURCE_TARBALL.tmp"
    mv "$SOURCE_DIR/$SOURCE_TARBALL.tmp" "$SOURCE_DIR/$SOURCE_TARBALL"
fi
BUILD_DIR=".build-brotli"
(test -d $BUILD_DIR && rm -rf $BUILD_DIR) || true
mkdir -p $BUILD_DIR
tar xf "$SOURCE_DIR/$SOURCE_TARBALL" -C $BUILD_DIR --strip 1
cd $BUILD_DIR
apply_patch brotli-${BROTLI_VERSION}

mkdir build && cd build
# Define -DBROTLI_EMSCRIPTEN=1 to disable shared libs
cmake .. $CMAKE_FLAGS -DBROTLI_EMSCRIPTEN=1 -DBROTLI_DISABLE_TESTS=TRUE
cmake --build .
cp libbrotlicommon-static.a "$OUTPUT_SYSROOT_LIBDIR/libbrotlicommon.a"
cp libbrotlidec-static.a "$OUTPUT_SYSROOT_LIBDIR/libbrotlidec.a"
cp libbrotlienc-static.a "$OUTPUT_SYSROOT_LIBDIR/libbrotlienc.a"
cp -r ../c/include/brotli "$OUTPUT_SYSROOT/include/"
