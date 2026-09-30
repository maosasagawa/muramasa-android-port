#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
NDK="${ANDROID_NDK_ROOT:-${ANDROID_NDK_HOME:-}}"
SDL_SOURCE="${SDL2_SOURCE_DIR:-}"
PNG_SOURCE="${LIBPNG_SOURCE_DIR:-}"
JPEG_SOURCE="${LIBJPEG_TURBO_SOURCE_DIR:-}"

if [[ -z "$NDK" || -z "$SDL_SOURCE" || -z "$PNG_SOURCE" || -z "$JPEG_SOURCE" ]]; then
    echo "Set ANDROID_NDK_ROOT, SDL2_SOURCE_DIR, LIBPNG_SOURCE_DIR, and LIBJPEG_TURBO_SOURCE_DIR." >&2
    exit 2
fi

TOOLCHAIN="$NDK/build/cmake/android.toolchain.cmake"
BUILD_ROOT="$ROOT/.build/android-xlibs"

for ABI in arm64-v8a x86_64; do
    PREFIX="$ROOT/android/xlibs/$ABI"
    BUILD_TYPE=()
    if [[ "$ABI" == arm64-v8a ]]; then
        BUILD_TYPE=(-DCMAKE_BUILD_TYPE=Release)
    fi

    PNG_BUILD="$BUILD_ROOT/libpng-$ABI"
    cmake -S "$PNG_SOURCE" -B "$PNG_BUILD" \
        -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN" \
        -DANDROID_ABI="$ABI" \
        -DANDROID_PLATFORM=android-24 \
        -DCMAKE_INSTALL_PREFIX="$PREFIX" \
        "${BUILD_TYPE[@]}" \
        -DPNG_BUILD_ZLIB=OFF \
        -DPNG_SHARED=OFF \
        -DPNG_STATIC=ON \
        -DPNG_TESTS=OFF \
        -DPNG_TOOLS=OFF
    cmake --build "$PNG_BUILD" --parallel "${JOBS:-$(nproc)}"
    cmake --install "$PNG_BUILD"

    JPEG_BUILD="$BUILD_ROOT/libjpeg-turbo-$ABI"
    cmake -S "$JPEG_SOURCE" -B "$JPEG_BUILD" \
        -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN" \
        -DANDROID_ABI="$ABI" \
        -DANDROID_PLATFORM=android-24 \
        -DCMAKE_INSTALL_PREFIX="$PREFIX" \
        "${BUILD_TYPE[@]}" \
        -DENABLE_SHARED=OFF \
        -DENABLE_STATIC=ON \
        -DWITH_JAVA=OFF \
        -DWITH_SIMD=OFF \
        -DWITH_TURBOJPEG=OFF
    cmake --build "$JPEG_BUILD" --parallel "${JOBS:-$(nproc)}"
    cmake --install "$JPEG_BUILD"

    SDL_BUILD="$BUILD_ROOT/sdl2-$ABI"
    # No SDL2 CMake cache or shell history was available; these configure options are inferred.
    cmake -S "$SDL_SOURCE" -B "$SDL_BUILD" \
        -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN" \
        -DANDROID_ABI="$ABI" \
        -DANDROID_PLATFORM=android-24 \
        -DCMAKE_INSTALL_PREFIX="$PREFIX" \
        "${BUILD_TYPE[@]}" \
        -DSDL_SHARED=ON \
        -DSDL_STATIC=OFF \
        -DSDL_TEST=OFF \
        -DSDL_TEST_LIBRARY=OFF
    cmake --build "$SDL_BUILD" --parallel "${JOBS:-$(nproc)}"
    cmake --install "$SDL_BUILD"
done
