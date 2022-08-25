#!/bin/bash
set -e
cd "$(dirname "$0")"
source config.sh

SOURCE_TARBALL=xz-${XZ_VERSION}.tar.gz
mkdir -p "$SOURCE_DIR"
if [ ! -f "$SOURCE_DIR/$SOURCE_TARBALL" ]; then
    curl -sSL "https://tukaani.org/xz/xz-${XZ_VERSION}.tar.gz" -o "$SOURCE_DIR/$SOURCE_TARBALL.tmp"
    mv "$SOURCE_DIR/$SOURCE_TARBALL.tmp" "$SOURCE_DIR/$SOURCE_TARBALL"
fi
BUILD_DIR=".build-lzma"
(test -d $BUILD_DIR && rm -rf $BUILD_DIR) || true
mkdir -p $BUILD_DIR
tar xf "$SOURCE_DIR/$SOURCE_TARBALL" -C $BUILD_DIR --strip 1
cd $BUILD_DIR
apply_patch xz-${XZ_VERSION}

./configure $AUTOCONF_FLAGS --disable-xz --disable-xzdec --disable-doc
make install -j$(cpu_count)
