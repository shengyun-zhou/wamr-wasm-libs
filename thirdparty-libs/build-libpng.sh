#!/bin/bash
set -e
cd "$(dirname "$0")"
source config.sh

SOURCE_TARBALL=libpng-${LIBPNG_VERSION}.tar.gz
mkdir -p "$SOURCE_DIR"
if [ ! -f "$SOURCE_DIR/$SOURCE_TARBALL" ]; then
    curl -sSL "https://download.sourceforge.net/libpng/libpng-${LIBPNG_VERSION}.tar.gz" -o "$SOURCE_DIR/$SOURCE_TARBALL.tmp"
    mv "$SOURCE_DIR/$SOURCE_TARBALL.tmp" "$SOURCE_DIR/$SOURCE_TARBALL"
fi
BUILD_DIR=".build-libpng"
(test -d $BUILD_DIR && rm -rf $BUILD_DIR) || true
mkdir -p $BUILD_DIR
tar xf "$SOURCE_DIR/$SOURCE_TARBALL" -C $BUILD_DIR --strip 1
cd $BUILD_DIR
apply_patch libpng-${LIBPNG_VERSION}

mkdir build && cd build
cmake .. $CMAKE_FLAGS -DPNG_SHARED=OFF -DPNG_STATIC=ON -DPNG_TESTS=OFF -DM_LIBRARY="" -DAWK=""

# Fix compile error
echo "#undef PNG_SETJMP_SUPPORTED" >> pnglibconf.h
echo "#undef PNG_SIMPLIFIED_WRITE_SUPPORTED" >> pnglibconf.h
echo "#undef PNG_SIMPLIFIED_READ_SUPPORTED" >> pnglibconf.h

cmake --build . --target install
rm -rf "$OUTPUT_SYSROOT/lib/libpng" || true
