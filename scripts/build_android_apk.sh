#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT/android/app"
./gradlew assembleDebug -PEXCLUDE_NATIVE_LIBS
