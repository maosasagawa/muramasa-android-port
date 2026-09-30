#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
JOBS="${JOBS:-$(nproc)}"

cmake -S "$ROOT/libnpa" -B "$ROOT/libnpa/build"
cmake --build "$ROOT/libnpa/build" --parallel "$JOBS"

cmake -S "$ROOT/libnpengine" -B "$ROOT/libnpengine/build" \
    -DNPA_INCLUDE_DIR="$ROOT/libnpa/include" \
    -DNPA_LIBRARY="$ROOT/libnpa/build/libnpa.so"
cmake --build "$ROOT/libnpengine/build" --parallel "$JOBS"

cmake -S "$ROOT/muramasa" -B "$ROOT/muramasa/build" \
    -DNPA_INCLUDE_DIR="$ROOT/libnpa/include" \
    -DNPA_LIBRARY="$ROOT/libnpa/build/libnpa.so" \
    -DNPENGINE_INCLUDE_DIR="$ROOT/libnpengine/include" \
    -DNPENGINE_LIBRARY="$ROOT/libnpengine/build/libnpengine.so"
cmake --build "$ROOT/muramasa/build" --parallel "$JOBS"
