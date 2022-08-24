#!/bin/bash
set -e
cd "$(dirname "$0")"
source config.sh

SOURCE_TARBALL=zlib-${ZLIB_VERSION}.tar.gz
mkdir -p "$SOURCE_DIR"
if [ ! -f "$SOURCE_DIR/$SOURCE_TARBALL" ]; then
    curl -sSL "https://${GITHUB_MIRROR_DOMAIN:-github.com}/madler/zlib/archive/refs/tags/v${ZLIB_VERSION}.tar.gz" -o "$SOURCE_DIR/$SOURCE_TARBALL.tmp"
    mv "$SOURCE_DIR/$SOURCE_TARBALL.tmp" "$SOURCE_DIR/$SOURCE_TARBALL"
fi
BUILD_DIR=".build-zlib"
(test -d $BUILD_DIR && rm -rf $BUILD_DIR) || true
mkdir -p $BUILD_DIR
tar xf "$SOURCE_DIR/$SOURCE_TARBALL" -C $BUILD_DIR --strip 1
cd $BUILD_DIR
apply_patch zlib-${ZLIB_VERSION}

./configure --prefix="$OUTPUT_SYSROOT" --libdir="$OUTPUT_SYSROOT/$SYSROOT_LIBDIR_PREFIX" --static
make install -j$(cpu_count)
