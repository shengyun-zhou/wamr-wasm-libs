#!/bin/bash
set -e
cd "$(dirname "$0")"
source config.sh

SOURCE_TARBALL=sqlite3-${SQLITE3_VERSION}.tar.gz
mkdir -p "$SOURCE_DIR"
if [ ! -f "$SOURCE_DIR/$SOURCE_TARBALL" ]; then
    curl -sSL "https://${GITHUB_MIRROR_DOMAIN:-github.com}/sqlite/sqlite/archive/refs/tags/version-${SQLITE3_VERSION}.tar.gz" -o "$SOURCE_DIR/$SOURCE_TARBALL.tmp"
    mv "$SOURCE_DIR/$SOURCE_TARBALL.tmp" "$SOURCE_DIR/$SOURCE_TARBALL"
fi
BUILD_DIR=".build-sqlite3"
(test -d $BUILD_DIR && rm -rf $BUILD_DIR) || true
mkdir -p $BUILD_DIR
tar xf "$SOURCE_DIR/$SOURCE_TARBALL" -C $BUILD_DIR --strip 1
cd $BUILD_DIR
apply_patch sqlite3-${SQLITE3_VERSION}

CFLAGS="$CFLAGS -DSQLITE_MAX_MMAP_SIZE=0" ./configure $AUTOCONF_FLAGS --disable-tcl --disable-readline --disable-load-extension
make lib_install -j$(cpu_count)
cp sqlite3.h "$OUTPUT_SYSROOT/include/"
