#!/bin/sh

set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
PROJECT_DIR=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)
PLATFORMIO_HOME=${PLATFORMIO_HOME:-$HOME/.platformio}

if [ -n "${PLATFORMIO_BIN:-}" ]; then
  PIO_BIN="$PLATFORMIO_BIN"
elif [ -x "$PLATFORMIO_HOME/penv/bin/platformio" ]; then
  PIO_BIN="$PLATFORMIO_HOME/penv/bin/platformio"
elif command -v platformio >/dev/null 2>&1; then
  PIO_BIN=$(command -v platformio)
else
  echo "platformio not found. Set PLATFORMIO_BIN or install PlatformIO." >&2
  exit 1
fi

if [ -n "${PYTHON_BIN:-}" ]; then
  PYTHON_BIN="$PYTHON_BIN"
elif [ -x "$PLATFORMIO_HOME/penv/bin/python" ]; then
  PYTHON_BIN="$PLATFORMIO_HOME/penv/bin/python"
elif command -v python3 >/dev/null 2>&1; then
  PYTHON_BIN=$(command -v python3)
else
  echo "python3 not found." >&2
  exit 1
fi

ESPPY=${ESPTOOL_PY:-$PLATFORMIO_HOME/packages/tool-esptoolpy/esptool.py}

FRAMEWORK_DIR=${ARDUINO_ESP32_DIR:-$PLATFORMIO_HOME/packages/framework-arduinoespressif32@3.20004.0}
if [ ! -d "$FRAMEWORK_DIR" ] && [ -d "$PLATFORMIO_HOME/packages/framework-arduinoespressif32" ]; then
  FRAMEWORK_DIR="$PLATFORMIO_HOME/packages/framework-arduinoespressif32"
fi

ENV_NAME=${PLATFORMIO_ENV:-m5stack-fire}
BUILD_DIR="$PROJECT_DIR/.pio/build/$ENV_NAME"
OUTPUT_PATH=${1:-"$BUILD_DIR/m5-paper-clock-complete.bin"}

BOOTLOADER_BIN="$FRAMEWORK_DIR/tools/sdk/esp32/bin/bootloader_dio_40m.bin"
BOOT_APP0_BIN="$FRAMEWORK_DIR/tools/partitions/boot_app0.bin"
PARTITIONS_BIN="$BUILD_DIR/partitions.bin"
FIRMWARE_BIN="$BUILD_DIR/firmware.bin"
SPIFFS_BIN="$BUILD_DIR/spiffs.bin"

cd "$PROJECT_DIR"

echo "Project: $PROJECT_DIR"
echo "PlatformIO: $PIO_BIN"
echo "Python: $PYTHON_BIN"
echo "Environment: $ENV_NAME"
echo "Output: $OUTPUT_PATH"

echo "==> 1/3 Building firmware..."
"$PIO_BIN" run -e "$ENV_NAME"

echo "==> 2/3 Building SPIFFS image..."
"$PIO_BIN" run -e "$ENV_NAME" -t buildfs

if [ ! -f "$ESPPY" ]; then
  echo "esptool.py not found: $ESPPY" >&2
  exit 1
fi

if [ ! -d "$FRAMEWORK_DIR" ]; then
  echo "framework-arduinoespressif32 not found: $FRAMEWORK_DIR" >&2
  exit 1
fi

for required in \
  "$BOOTLOADER_BIN" \
  "$BOOT_APP0_BIN"; do
  if [ ! -f "$required" ]; then
    echo "Required file not found: $required" >&2
    exit 1
  fi
done

for required in \
  "$PARTITIONS_BIN" \
  "$FIRMWARE_BIN" \
  "$SPIFFS_BIN"; do
  if [ ! -f "$required" ]; then
    echo "Expected build artifact not found: $required" >&2
    exit 1
  fi
done

mkdir -p "$(dirname "$OUTPUT_PATH")"

echo "==> 3/3 Merging flash image..."
"$PYTHON_BIN" "$ESPPY" --chip esp32 merge_bin \
  -o "$OUTPUT_PATH" \
  --fill-flash-size 16MB \
  --flash_mode dio \
  --flash_freq 40m \
  --flash_size 16MB \
  0x1000 "$BOOTLOADER_BIN" \
  0x8000 "$PARTITIONS_BIN" \
  0xe000 "$BOOT_APP0_BIN" \
  0x10000 "$FIRMWARE_BIN" \
  0x810000 "$SPIFFS_BIN"

echo "Done."
echo "$OUTPUT_PATH"
