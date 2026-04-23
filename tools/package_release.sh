#!/bin/sh

set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
PROJECT_DIR=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)
ENV_NAME=${PLATFORMIO_ENV:-m5stack-fire}

if [ -n "${FIRMWARE_VERSION:-}" ]; then
  VERSION="$FIRMWARE_VERSION"
else
  VERSION=$(git -C "$PROJECT_DIR" describe --tags --dirty --always 2>/dev/null || printf 'dev')
fi

BUILD_SHA=${FIRMWARE_GIT_SHA:-$(git -C "$PROJECT_DIR" rev-parse --short=12 HEAD 2>/dev/null || printf 'unknown')}
BUILD_TIME=${FIRMWARE_BUILD_TIME:-$(date -u +"%Y-%m-%dT%H:%M:%SZ")}
SAFE_VERSION=$(printf '%s' "$VERSION" | tr '/: ' '---')
DIST_DIR=${DIST_DIR:-"$PROJECT_DIR/dist/m5-paper-clock-$SAFE_VERSION"}
BUILD_DIR="$PROJECT_DIR/.pio/build/$ENV_NAME"

export FIRMWARE_VERSION="$VERSION"
export FIRMWARE_GIT_SHA="$BUILD_SHA"
export FIRMWARE_BUILD_TIME="$BUILD_TIME"

rm -rf "$DIST_DIR"
mkdir -p "$DIST_DIR"

"$SCRIPT_DIR/build_merged_image.sh" "$DIST_DIR/m5-paper-clock-complete.bin"

cp "$BUILD_DIR/firmware.bin" "$DIST_DIR/firmware.bin"
cp "$BUILD_DIR/spiffs.bin" "$DIST_DIR/spiffs.bin"
cp "$BUILD_DIR/partitions.bin" "$DIST_DIR/partitions.bin"

export RELEASE_DIST_DIR="$DIST_DIR"
export RELEASE_VERSION="$VERSION"
export RELEASE_BUILD_SHA="$BUILD_SHA"
export RELEASE_BUILD_TIME="$BUILD_TIME"

node "$SCRIPT_DIR/write_release_metadata.mjs"

echo "Release package:"
echo "$DIST_DIR"
