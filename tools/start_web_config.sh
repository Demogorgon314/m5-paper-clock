#!/bin/sh

set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
PROJECT_DIR=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)
WEB_DIR="$PROJECT_DIR/web"
PORT="${1:-4173}"

if ! command -v python3 >/dev/null 2>&1; then
  echo "python3 not found." >&2
  exit 1
fi

if [ ! -d "$WEB_DIR" ]; then
  echo "web directory not found: $WEB_DIR" >&2
  exit 1
fi

echo "Serving Web Serial config UI at http://localhost:$PORT"
echo "Press Ctrl+C to stop."

cd "$PROJECT_DIR"
exec python3 -m http.server "$PORT" --directory "$WEB_DIR"
