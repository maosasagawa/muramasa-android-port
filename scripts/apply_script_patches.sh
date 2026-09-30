#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PATCH_FILE="$ROOT/patches/sys_config.nss.patch"
GAME_DIR=""
ARCHIVE=""
OUTPUT=""

usage() {
    echo "Usage: $0 (--game-dir DIR | --npa FILE) --output FILE" >&2
}

while (($#)); do
    case "$1" in
        --game-dir) GAME_DIR="$2"; shift 2 ;;
        --npa) ARCHIVE="$2"; shift 2 ;;
        --output) OUTPUT="$2"; shift 2 ;;
        -h|--help) usage; exit 0 ;;
        *) usage; exit 2 ;;
    esac
done

if [[ -z "$OUTPUT" || ( -z "$GAME_DIR" && -z "$ARCHIVE" ) || ( -n "$GAME_DIR" && -n "$ARCHIVE" ) ]]; then
    usage
    exit 2
fi

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT
ORIGINAL="$WORK/sys_config.nss"

if [[ -n "$GAME_DIR" ]]; then
    cp "$GAME_DIR/sys_config.nss" "$WORK/original.nss"
else
    ARCHIVE="$(cd "$(dirname "$ARCHIVE")" && pwd)/$(basename "$ARCHIVE")"
    NPAEXTRACT="${NPAEXTRACT_BIN:-npaextract}"
    if ! command -v "$NPAEXTRACT" >/dev/null 2>&1; then
        echo "npaextract was not found; set NPAEXTRACT_BIN or add it to PATH." >&2
        exit 2
    fi
    (cd "$WORK" && "$NPAEXTRACT" "$ARCHIVE" Muramasa)
    EXTRACTED="$(find "$WORK" -type f -name sys_config.nss -print -quit)"
    if [[ -z "$EXTRACTED" ]]; then
        echo "sys_config.nss was not found in the NPA archive." >&2
        exit 1
    fi
    cp "$EXTRACTED" "$WORK/original.nss"
fi

iconv -f GBK -t UTF-8 "$WORK/original.nss" | sed 's/\r$//' > "$ORIGINAL"
patch --silent --batch -p1 -d "$WORK" < "$PATCH_FILE"
mkdir -p "$(dirname "$OUTPUT")"
iconv -f UTF-8 -t GBK "$ORIGINAL" > "$OUTPUT"
