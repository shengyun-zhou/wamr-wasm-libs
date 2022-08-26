__PRE_CWD="$(pwd)"
cd ..
source tools.sh
source version
cd "$__PRE_CWD"

export CFLAGS="-O2 -g -ffunction-sections -fdata-sections -I$OUTPUT_SYSROOT/include"
export CXXFLAGS="$CFLAGS"
export CPPFLAGS="-I$OUTPUT_SYSROOT/include"
export LDFLAGS="-L$OUTPUT_SYSROOT_LIBDIR"
export CC=${CROSS_PREFIX}-clang
export CXX=${CROSS_PREFIX}-clang++
export CPP="${CC} --driver-mode=cpp"
export AR=${CROSS_PREFIX}-ar
export RANLIB=${CROSS_PREFIX}-ranlib
export STRIP=${CROSS_PREFIX}-strip
# Pretend to be Emscripten for autoconf
export AUTOCONF_FLAGS="--host=wasm32-emscripten --prefix=$OUTPUT_SYSROOT --libdir=$OUTPUT_SYSROOT_LIBDIR
  --enable-static --disable-shared"

export CMAKE_FLAGS="-G Ninja -DCMAKE_SYSTEM_NAME=WASI -DCMAKE_MODULE_PATH=$(pwd)/../cmake -DCMAKE_BUILD_TYPE=Release
  -DCMAKE_VERBOSE_MAKEFILE=1 -DCMAKE_INSTALL_PREFIX=$OUTPUT_SYSROOT -DCMAKE_INSTALL_LIBDIR=$SYSROOT_LIBDIR_PREFIX
  -DCMAKE_FIND_ROOT_PATH=$OUTPUT_SYSROOT"
