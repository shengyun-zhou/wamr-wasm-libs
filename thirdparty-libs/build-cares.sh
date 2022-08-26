#!/bin/bash
set -e
cd "$(dirname "$0")"
source config.sh

SOURCE_TARBALL=cares-${CARES_VERSION}.tar.gz
mkdir -p "$SOURCE_DIR"
if [ ! -f "$SOURCE_DIR/$SOURCE_TARBALL" ]; then
    curl -sSL "https://c-ares.org/download/c-ares-${CARES_VERSION}.tar.gz" -o "$SOURCE_DIR/$SOURCE_TARBALL.tmp"
    mv "$SOURCE_DIR/$SOURCE_TARBALL.tmp" "$SOURCE_DIR/$SOURCE_TARBALL"
fi
BUILD_DIR=".build-cares"
(test -d $BUILD_DIR && rm -rf $BUILD_DIR) || true
mkdir -p $BUILD_DIR
tar xf "$SOURCE_DIR/$SOURCE_TARBALL" -C $BUILD_DIR --strip 1
cd $BUILD_DIR
apply_patch cares-${CARES_VERSION}

mkdir build && cd build
cmake .. $CMAKE_FLAGS -DCARES_STATIC=ON -DCARES_SHARED=OFF -DCARES_BUILD_TOOLS=OFF
cmake --build . --target install
