#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
"$SCRIPT_DIR/build_android_xlibs.sh"
"$SCRIPT_DIR/build_android_native.sh"
"$SCRIPT_DIR/build_android_apk.sh"
