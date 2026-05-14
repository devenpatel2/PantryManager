#!/usr/bin/env bash
# Created by Claude: one-time setup of the PantryManager FastAPI server on the Pi.
# - Verifies uv is installed
# - Creates / syncs the project venv via `uv sync`
# - Installs the systemd user unit and starts the service
# - Enables linger so the service runs on boot without a login session
# Safe to re-run (idempotent).
set -euo pipefail

REPO_DIR=$(cd "$(dirname "$0")/.." && pwd)
SERVER_DIR=$REPO_DIR/PantryServer
UNIT_SRC=$REPO_DIR/scripts/systemd/pantry-server.service
UNIT_DST=$HOME/.config/systemd/user/pantry-server.service

echo "=== Checking uv ==="
if ! command -v uv >/dev/null 2>&1; then
    echo "uv is not installed. Install it with:"
    echo "  curl -LsSf https://astral.sh/uv/install.sh | sh"
    echo "Then re-run this script."
    exit 1
fi
uv --version

echo "=== Syncing project venv ($SERVER_DIR/.venv) ==="
cd "$SERVER_DIR"
uv sync

echo "=== Installing systemd user unit ==="
mkdir -p "$(dirname "$UNIT_DST")"
cp "$UNIT_SRC" "$UNIT_DST"

echo "=== Reloading + enabling service ==="
systemctl --user daemon-reload
systemctl --user enable --now pantry-server

echo "=== Enabling linger (so service runs on boot without login) ==="
if loginctl show-user "$USER" 2>/dev/null | grep -q "Linger=yes"; then
    echo "Linger already enabled."
else
    sudo loginctl enable-linger "$USER"
fi

echo
echo "=== Status ==="
systemctl --user status pantry-server --no-pager -n 5 || true

echo
echo "Done. Useful commands:"
echo "  systemctl --user status pantry-server"
echo "  systemctl --user restart pantry-server"
echo "  journalctl --user -u pantry-server -f"
