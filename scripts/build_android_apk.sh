#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ASSETS="$ROOT/android/app/app/src/main/assets"
if [[ -d "$ASSETS/gamedata" ]]; then
    (cd "$ASSETS/gamedata" && find . -type f ! -name manifest.txt -printf '%s\t%P\n' | LC_ALL=C sort > manifest.txt)
fi
cd "$ROOT/android/app"
./gradlew assembleDebug -PEXCLUDE_NATIVE_LIBS
