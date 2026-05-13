#!/usr/bin/env bash
# Created by Claude: compiles on laptop, rsyncs binary to Pi, flashes, captures serial.
# Usage: deploy.sh [monitor_seconds]
set -e

# arduino-cli lives in ~/.local/bin on the laptop, not in the default non-interactive PATH
export PATH="$HOME/.local/bin:$PATH"

REPO_DIR=$(cd "$(dirname "$0")/.." && pwd)
SKETCH_DIR=$REPO_DIR/PantryClient
BUILD_DIR=$REPO_DIR/build
PI_HOST=pi3-wifi
PI_BIN_PATH=/tmp/PantryClient.ino.bin
SERIAL_PORT=/dev/ttyUSB0
FQBN=esp8266:esp8266:nodemcu
MONITOR_DURATION=${1:-20}

echo "=== Compiling on laptop ==="
arduino-cli compile -j4 --fqbn "$FQBN" --output-dir "$BUILD_DIR" "$SKETCH_DIR"

echo "=== Rsyncing binary to $PI_HOST ==="
rsync -az "$BUILD_DIR/PantryClient.ino.bin" "$PI_HOST:$PI_BIN_PATH"

echo "=== Flashing + monitoring on $PI_HOST (${MONITOR_DURATION}s) ==="
ssh "$PI_HOST" "export PATH=\$HOME/.local/bin:\$PATH; \
  arduino-cli upload --input-file $PI_BIN_PATH -p $SERIAL_PORT --fqbn $FQBN \
  && sleep 1 \
  && timeout $MONITOR_DURATION arduino-cli monitor -p $SERIAL_PORT --config baudrate=9600 || true"
