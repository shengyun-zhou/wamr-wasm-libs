#!/bin/bash
set -e
cd "$(dirname "$0")"
source config.sh

SOURCE_TARBALL=curl-${CURL_VERSION}.tar.gz
mkdir -p "$SOURCE_DIR"
if [ ! -f "$SOURCE_DIR/$SOURCE_TARBALL" ]; then
    curl -sSL "https://curl.se/download/curl-${CURL_VERSION}.tar.gz" -o "$SOURCE_DIR/$SOURCE_TARBALL.tmp"
    mv "$SOURCE_DIR/$SOURCE_TARBALL.tmp" "$SOURCE_DIR/$SOURCE_TARBALL"
fi
BUILD_DIR=".build-curl"
(test -d $BUILD_DIR && rm -rf $BUILD_DIR) || true
mkdir -p $BUILD_DIR
tar xf "$SOURCE_DIR/$SOURCE_TARBALL" -C $BUILD_DIR --strip 1
cd $BUILD_DIR
apply_patch curl-${CURL_VERSION}

mkdir build && cd build
CFLAGS="-DNGHTTP2_STATICLIB -DCARES_STATICLIB $CFLAGS" \
cmake .. $CMAKE_FLAGS -DBUILD_SHARED_LIBS=OFF \
    -DCURL_USE_OPENSSL=ON -DCURL_ZLIB=ON -DENABLE_ARES=ON -DUSE_NGHTTP2=ON \
    -DCURL_ZSTD=ON -DCURL_BROTLI=ON -DCMAKE_USE_LIBSSH2=OFF -DHAVE_POLL_FINE=1
cmake --build . --target install
