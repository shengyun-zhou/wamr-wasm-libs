#!/bin/bash

__os_name="$(uname -s)"
case "$__os_name" in
    CYGWIN*|MINGW*|MSYS*)
        export BUILD_HOST=Windows
        ;;
    Linux*)
        export BUILD_HOST=Linux
        ;;
    Darwin*)
        export BUILD_HOST=Darwin
        ;;
esac
unset __os_name
export PATCH_DIR="$(pwd)/patch"
export SOURCE_DIR="${SOURCE_DIR:-"$(pwd)/.src-tarballs"}"
export CROSS_PREFIX="${CROSS_PREFIX:-wasm32-wamr-wasi-pthread}"
export OUTPUT_SYSROOT="$(pwd)/sysroot"
export SYSROOT_LIBDIR_PREFIX='lib/wasm32-wasi-pthread'
export SYSROOT_OLD_LIBDIR_PREFIX='lib/wasm32-wasi'
export OUTPUT_SYSROOT_LIBDIR="$OUTPUT_SYSROOT/$SYSROOT_LIBDIR_PREFIX"

function apply_patch {
    if [ -d "$PATCH_DIR/$1" ]; then
        for patchfile in "$PATCH_DIR/$1/"*.patch; do
            patch -p1 < "$patchfile"
        done
    fi
}

function cpu_count {
    if [[ $BUILD_HOST == "Darwin" ]]; then
        NCPU="$(sysctl -n hw.logicalcpu)"
    else
        NCPU="$(nproc)"
    fi
    echo "$NCPU"
}
