#!/bin/bash
set -e
cd "$(dirname "$0")"
source config.sh

SOURCE_TARBALL=openssl-${OPENSSL_VERSION}.tar.gz
mkdir -p "$SOURCE_DIR"
if [ ! -f "$SOURCE_DIR/$SOURCE_TARBALL" ]; then
    curl -sSL "https://www.openssl.org/source/openssl-${OPENSSL_VERSION}.tar.gz" -o "$SOURCE_DIR/$SOURCE_TARBALL.tmp"
    mv "$SOURCE_DIR/$SOURCE_TARBALL.tmp" "$SOURCE_DIR/$SOURCE_TARBALL"
fi
BUILD_DIR=".build-openssl"
(test -d $BUILD_DIR && rm -rf $BUILD_DIR) || true
mkdir -p $BUILD_DIR
tar xf "$SOURCE_DIR/$SOURCE_TARBALL" -C $BUILD_DIR --strip 1
cd $BUILD_DIR
apply_patch openssl-${OPENSSL_VERSION}

./Configure linux-generic32 --prefix="$OUTPUT_SYSROOT" --libdir="$OUTPUT_SYSROOT_LIBDIR" \
  no-dso no-shared no-tests no-ui-console no-engine
make install_dev -j$(cpu_count)
