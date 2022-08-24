#!/bin/bash
set -e
cd "$(dirname "$0")"
source config.sh

SOURCE_TARBALL=bzip2-${BZIP2_VERSION}.tar.gz
mkdir -p "$SOURCE_DIR"
if [ ! -f "$SOURCE_DIR/$SOURCE_TARBALL" ]; then
    curl -sSL "https://sourceware.org/pub/bzip2/bzip2-${BZIP2_VERSION}.tar.gz" -o "$SOURCE_DIR/$SOURCE_TARBALL.tmp"
    mv "$SOURCE_DIR/$SOURCE_TARBALL.tmp" "$SOURCE_DIR/$SOURCE_TARBALL"
fi
BUILD_DIR=".build-bzip2"
(test -d $BUILD_DIR && rm -rf $BUILD_DIR) || true
mkdir -p $BUILD_DIR
tar xf "$SOURCE_DIR/$SOURCE_TARBALL" -C $BUILD_DIR --strip 1
cd $BUILD_DIR
apply_patch bzip2-${BZIP2_VERSION}

make CC=$CC AR=$AR RANLIB=$RANLIB CFLAGS="$CFLAGS -Wall -Winline -D_FILE_OFFSET_BITS=64" LDFLAGS="$LDFLAGS" \
 -j$(cpu_count) libbz2.a bzip2 bzip2recover
make install PREFIX="$(pwd)/bzip2-install"
cp -r bzip2-install/include/* "$OUTPUT_SYSROOT/include/"
cp -r bzip2-install/lib/* "$OUTPUT_SYSROOT/$SYSROOT_LIBDIR_PREFIX/"
