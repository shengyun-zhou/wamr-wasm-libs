__PRE_CWD="$(pwd)"
cd ..
source tools.sh
source version
cd "$__PRE_CWD"

export CFLAGS="-O2 -g -ffunction-sections -fdata-sections -I$OUTPUT_SYSROOT/include"
export CXXFLAGS="$CFLAGS"
export CPPFLAGS="--driver-mode=cpp -I$OUTPUT_SYSROOT/include"
export LDFLAGS="-L$OUTPUT_SYSROOT_LIBDIR"
export CC=${CROSS_PREFIX}-clang
export CXX=${CROSS_PREFIX}-clang++
export CPP=${CC}
export AR=${CROSS_PREFIX}-ar
export RANLIB=${CROSS_PREFIX}-ranlib
export STRIP=${CROSS_PREFIX}-strip
# Pretend to be Emscripten for autoconf
export AUTOCONF_FLAGS="--host=wasm32-emscripten --prefix=$OUTPUT_SYSROOT --libdir=$OUTPUT_SYSROOT/$SYSROOT_LIBDIR_PREFIX
  --enable-static --disable-shared"
