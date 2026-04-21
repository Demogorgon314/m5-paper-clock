#!/bin/sh

set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
PROJECT_DIR=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)

if [ -n "${PLATFORMIO_BIN:-}" ]; then
  PIO_BIN="$PLATFORMIO_BIN"
elif [ -x "/Users/wangkai/.platformio/penv/bin/platformio" ]; then
  PIO_BIN="/Users/wangkai/.platformio/penv/bin/platformio"
elif command -v platformio >/dev/null 2>&1; then
  PIO_BIN=$(command -v platformio)
else
  echo "platformio not found. Set PLATFORMIO_BIN or install PlatformIO." >&2
  exit 1
fi

ENV_NAME=${PLATFORMIO_ENV:-m5stack-fire}
REQUESTED_PORT=${1:-${PORT:-}}

port_exists() {
  [ -n "${1:-}" ] && [ -e "$1" ]
}

find_port() {
  for candidate in \
    /dev/cu.usbserial* \
    /dev/cu.SLAB_USBtoUART* \
    /dev/cu.wchusbserial* \
    /dev/cu.usbmodem*; do
    if [ -e "$candidate" ]; then
      printf '%s\n' "$candidate"
      return 0
    fi
  done
  return 1
}

resolve_port() {
  if port_exists "$REQUESTED_PORT"; then
    printf '%s\n' "$REQUESTED_PORT"
    return 0
  fi

  find_port || true
}

PORT=$(resolve_port)
if [ -z "$PORT" ]; then
  echo "No serial port found. Pass one explicitly: tools/flash_all.sh /dev/cu.usbserial-XXXX" >&2
  exit 1
fi

run_target() {
  target=$1
  attempt=1
  while [ "$attempt" -le 3 ]; do
    PORT=$(resolve_port)
    if [ -z "$PORT" ]; then
      echo "No serial port available for target '$target'." >&2
      if [ "$attempt" -ge 3 ]; then
        return 1
      fi
      echo "Waiting for serial reconnect before '$target'..." >&2
      attempt=$((attempt + 1))
      sleep 2
      continue
    fi

    echo "Port: $PORT"
    if "$PIO_BIN" run -e "$ENV_NAME" -t "$target" --upload-port "$PORT"; then
      return 0
    fi

    if [ "$attempt" -ge 3 ]; then
      return 1
    fi

    echo "Retrying '$target' after serial reconnect..." >&2
    attempt=$((attempt + 1))
    sleep 3
  done
}

echo "Project: $PROJECT_DIR"
echo "PlatformIO: $PIO_BIN"
echo "Environment: $ENV_NAME"
echo "Port: $PORT"

cd "$PROJECT_DIR"

echo "==> 1/2 Uploading SPIFFS filesystem (font)..."
run_target uploadfs

echo "==> 2/2 Uploading firmware..."
run_target upload

echo "Done."
