#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
NDK="${ANDROID_NDK_ROOT:-${ANDROID_NDK_HOME:-}}"
XLIBS_ROOT="${XLIBS_ROOT:-$ROOT/android/xlibs}"
if [[ -z "$NDK" ]]; then
    echo "Set ANDROID_NDK_ROOT (or ANDROID_NDK_HOME)." >&2
    exit 2
fi

TOOLCHAIN="$NDK/build/cmake/android.toolchain.cmake"
for ABI in arm64-v8a x86_64; do
    BUILD="$ROOT/.build/android-native-$ABI"
    OUTPUT="$ROOT/android/app/app/src/main/jniLibs/$ABI"
    cmake -S "$ROOT/android/native" -B "$BUILD" \
        -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN" \
        -DANDROID_ABI="$ABI" \
        -DANDROID_PLATFORM=android-24 \
        -DCMAKE_BUILD_TYPE=Release \
        -DXLIBS_ROOT="$XLIBS_ROOT"
    cmake --build "$BUILD" --parallel "${JOBS:-$(nproc)}"

    mkdir -p "$OUTPUT"
    cp "$BUILD/libmain.so" "$OUTPUT/libmain.so"
    cp "$XLIBS_ROOT/$ABI/lib/libSDL2.so" "$OUTPUT/libSDL2.so"
done
