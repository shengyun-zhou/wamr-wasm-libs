#!/bin/bash
set -e
cd "$(dirname "$0")"
source tools.sh
source version

SOURCE_TARBALL=wasi-libc-${WASI_LIBC_COMMIT_ID:0:8}.tar.gz
mkdir -p "$SOURCE_DIR"
if [ ! -f "$SOURCE_DIR/$SOURCE_TARBALL" ]; then
    curl -sSL "https://${GITHUB_MIRROR_DOMAIN:-github.com}/WebAssembly/wasi-libc/archive/$WASI_LIBC_COMMIT_ID.tar.gz" -o "$SOURCE_DIR/$SOURCE_TARBALL.tmp"
    mv "$SOURCE_DIR/$SOURCE_TARBALL.tmp" "$SOURCE_DIR/$SOURCE_TARBALL"
fi
BUILD_DIR="wasi-libc"
if [[ -z "$__PRESERVE_WASI_LIBC_DIR" ]]; then
  (test -d $BUILD_DIR && rm -rf $BUILD_DIR) || true
  mkdir -p $BUILD_DIR
  tar xf "$SOURCE_DIR/$SOURCE_TARBALL" -C $BUILD_DIR --strip 1
  cd $BUILD_DIR
  apply_patch wasi-libc-${WASI_LIBC_COMMIT_ID:0:8}
  # Remove -Werror compile flag
  sed -i.bak "s/-Werror//g" Makefile
  # Remove symbol different checking
  sed -i.bak "/diff -wur/d" Makefile
  # Backup include dir
  cp -r libc-top-half/musl/include libc-top-half/musl/include.bak
else
  cd $BUILD_DIR
fi

export CC=${CC:-${CROSS_PREFIX}-clang}

make CC="$CC" AR="${CROSS_PREFIX}-ar" NM="${CROSS_PREFIX}-nm" THREAD_MODEL=posix MALLOC_IMPL=none -j$(cpu_count)

if [[ -n "$SYSROOT_OLD_LIBDIR_PREFIX" ]]; then
  ln -sfn "$(basename sysroot/$SYSROOT_LIBDIR_PREFIX)" "sysroot/$SYSROOT_OLD_LIBDIR_PREFIX"
fi

# Merge some emulated libs into libc
"${CROSS_PREFIX}-ar" qcsL "sysroot/$SYSROOT_LIBDIR_PREFIX/libc.a" "sysroot/$SYSROOT_LIBDIR_PREFIX/libwasi-emulated-signal.a"
"${CROSS_PREFIX}-ar" qcsL "sysroot/$SYSROOT_LIBDIR_PREFIX/libc.a" "sysroot/$SYSROOT_LIBDIR_PREFIX/libwasi-emulated-mman.a"

mkdir -p "$OUTPUT_SYSROOT"
cp -r sysroot/* "$OUTPUT_SYSROOT"

