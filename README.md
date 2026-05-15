# PantryManager

A small IoT system to keep tabs on what's in the kitchen pantry.

- **PantryClient**: ESP8266 (NodeMCU) firmware driving a 128×160 ST7735 TFT and
  a rotary encoder. Shows the item list, lets you cycle an item's status
  (available / unavailable / surplus) with the encoder click, and auto-switches
  to a weather screen after a minute of inactivity.
- **PantryServer**: FastAPI app holding the canonical item state in SQLite,
  serving a mobile-friendly web UI, and bridging changes between the board and
  the browser over MQTT.
- **MQTT broker**: Mosquitto on the Raspberry Pi, the message bus that keeps board and server in sync.

This is a personal/home project — single-user, home-LAN, no auth.

## Architecture

```
┌──────────────┐                        ┌──────────────┐
│  PantryClient│                        │   Browser    │
│  (NodeMCU +  │                        │ (laptop/phone│
│   TFT + enc.)│                        │   on LAN)    │
└──────┬───────┘                        └──────┬───────┘
       │ MQTT                                  │ HTTP
       │                                       │
       ▼                                       ▼
┌──────────────┐    MQTT     ┌────────────────────────┐
│   Mosquitto  │◀───────────▶│      PantryServer      │
│   (broker,   │             │  (FastAPI + SQLite +   │
│   on Pi)     │             │   Jinja2 + HTMX UI)    │
└──────────────┘             └────────────────────────┘
```

Topics:
- `/pantry/items` — server → client (add/update/remove, plus chunked bulk dumps)
- `/pantry/update` — client → server (encoder click status changes)
- `/pantry/request` — client → server (request a fresh bulk dump on connect)
- `/pantry/weather` — server → client (Open-Meteo data, published every 60 s)

## Hardware

NodeMCU (ESP8266) board with:
- ST7735 1.8" TFT (128×160) over hardware SPI
- KY-040 rotary encoder + an **external 10 kΩ pull-up** on the SW pin (important — see hardware doc)

## One-time setup

### MQTT broker (on the Pi)

```bash
cd /home/dpa/PantryManager/mqtt
docker compose up -d
```

See [`mqtt/README.md`](mqtt/README.md) for start/stop/logs.

### PantryServer (on the Pi)

```bash
# Install uv if not already present
curl -LsSf https://astral.sh/uv/install.sh | sh

cd /home/dpa/PantryManager
bash scripts/install-server.sh
```

The script creates the venv via `uv sync`, installs the systemd user unit, enables linger, and starts the service. Idempotent — safe to re-run.

### PantryClient (on the laptop)

```bash
cd PantryClient
cp wifi_credentials.example.h wifi_credentials.h
$EDITOR wifi_credentials.h     # fill in real SSID + password
```

Then with the NodeMCU plugged into the Pi:

```bash
arduino-deploy 30
```

`arduino-deploy` compiles locally, rsyncs the binary to the Pi, and flashes it over USB. See `~/.bash_aliases` for the function body.

## Daily workflow

| Task | Command |
|---|---|
| Pull latest on Pi | `ssh pi3-wifi && cd PantryManager && git pull` |
| Restart server after pull | `systemctl --user restart pantry-server` |
| Re-sync server deps after pull | `cd PantryServer && uv sync` |
| Flash a new client build | `arduino-deploy 30` (from laptop) |
| Tail server logs | `journalctl --user -u pantry-server -f` |
| Tail board serial | `picocom /dev/ttyUSB0 -b 9600` (on Pi) |
| Trace MQTT traffic | `mosquitto_sub -h localhost -t '/pantry/#' -v` (on Pi) |
| Open the web UI | `http://pi3-wifi.fritz.box:8080/` |

The `scripts/install-server.sh` script wraps git-pull-less re-sync + restart, so re-running it is a safe full re-deploy step.

## Repo layout

```
.
├── PantryClient/               # Arduino sketch + encoder/display/MQTT modules
│   ├── PantryClient.ino
│   ├── encoder.cpp / .h
│   ├── display.cpp / .h
│   ├── mqtt.cpp / .h
│   ├── itemManager.cpp / .h
│   ├── weatherManager.cpp / .h
│   ├── buttons.cpp / .h        # fallback input source (compile-time switch)
│   ├── input_config.h          # INPUT_ENCODER (default) or INPUT_BUTTONS
│   └── wifi_credentials.example.h
├── PantryServer/               # FastAPI app
│   ├── pyproject.toml
│   ├── uv.lock
│   └── src/
│       ├── main.py
│       ├── database.py
│       ├── mqtt_handler.py
│       ├── weather_handler.py
│       ├── helper.py
│       ├── models.py
│       ├── initial_items.yaml  # seeds + additive-merges on every startup
│       └── templates/          # Jinja2 + HTMX UI
├── mqtt/                       # broker definition (docker-compose + config)
├── scripts/                    # deploy.sh, install-server.sh, systemd unit
├── docs/                       # hardware.md and any future docs
├── electronics/                # schematic, PCB layout, BOM, assembly
├── enclosure/                  # OpenSCAD source + STL exports
├── Pictures/                   # reference photos (display, encoder, breadboard)
├── CLAUDE.md                   # guidance for Claude Code
├── about_project.md            # historical resume notes (Apr 2026)
└── README.md
```

## Links

- [`docs/hardware.md`](docs/hardware.md) — wiring + bill of materials
- [`electronics/`](electronics/) — schematic, PCB, BOM, assembly notes
- [`enclosure/`](enclosure/) — OpenSCAD source and STLs
- [`mqtt/README.md`](mqtt/README.md) — broker start/stop/logs
- [`CLAUDE.md`](CLAUDE.md) — repo-specific guidance for Claude Code
- [`about_project.md`](about_project.md) — year-ago notes on where the project was when paused (kept for context)
