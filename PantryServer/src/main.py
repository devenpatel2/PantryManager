import asyncio
import time
import yaml
from pathlib import Path
from fastapi import FastAPI, BackgroundTasks, Form, Request
from fastapi.responses import HTMLResponse, Response
from fastapi.templating import Jinja2Templates
from .mqtt_handler import MQTTManager
from .database import DatabaseManager
from .models import SingleItemMessage, BulkMessage
from .weather_handler import WeatherHandler

app = FastAPI()

# Configuration
DB_PATH = "src/items.db"
API_URL="https://api.open-meteo.com/v1/forecast"
LATITUDE=48.1147  # Munich - 81377
LONGITUDE=11.476975

# Managers
db_manager = DatabaseManager(DB_PATH)
mqtt_manager = MQTTManager(db_manager)
weather_handler = WeatherHandler(API_URL, LATITUDE, LONGITUDE)
tasks = []

templates = Jinja2Templates(directory=str(Path(__file__).parent / "templates"))

# Sort like the board: status 1 (out) first, then alphabetical.
def _sorted_rows(filter_status: str | None = None):
    rows = db_manager.fetch_all_with_timestamps()
    if filter_status == "unavailable":
        rows = [r for r in rows if r[1] == 1]
    elif filter_status == "available":
        rows = [r for r in rows if r[1] == 0]
    elif filter_status == "surplus":
        rows = [r for r in rows if r[1] == 2]
    rows.sort(key=lambda r: (0 if r[1] == 1 else 1, r[0].lower()))
    return rows

@app.on_event("startup")
async def startup():
    # Initialize database and MQTT
    with open("src/initial_items.yaml", "r") as f:
        initial_data = yaml.safe_load(f)["items"]
    db_manager.initialize(initial_data)
    await mqtt_manager.connect()
    await mqtt_manager.publish_bulk()
    await weather_handler.fetch_weather()
    tasks.append(asyncio.create_task(weather_handler.start_periodic_updates()))
    time_to_next_minute = 60 - int(time.strftime("%S"))
    tasks.append(asyncio.create_task(mqtt_manager.subscribe_to_topics()))
    await asyncio.sleep(time_to_next_minute)
    tasks.append(asyncio.create_task(schedule_weather_updates()))

@app.on_event("shutdown")
async def shutdown():
    await mqtt_manager.disconnect()
    for task in tasks:
        task.cancel()
        try:
            await task
        except asyncio.CancelledError:
            pass

async def schedule_weather_updates():
    """Fetch and publish weather updates periodically."""
    while True:
        await mqtt_manager.publish_weather(weather_handler.get_cached_weather())
        await asyncio.sleep(60)  # publish weather every minute

@app.get("/weather")
async def get_weather():
    """API endpoint to get the current weather data."""
    return weather_handler.get_cached_weather()

@app.post("/items")
async def handle_item(data: SingleItemMessage, background_tasks: BackgroundTasks):
    if data.op in ("add", "update"):
        db_manager.update_item(data.item, data.status)
    elif data.op == "remove":
        db_manager.delete_item(data.item)

    payload = {
        "item": data.item,
        "status": data.status,
        "op": data.op,
        "timestamp": data.timestamp,
    }
    background_tasks.add_task(mqtt_manager.publish_message, payload)
    return {"message": f"Item {data.op} operation successful"}

# ---- Web UI ----

@app.get("/", response_class=HTMLResponse)
async def index(request: Request, filter: str = "all"):
    rows = _sorted_rows(None if filter == "all" else filter)
    return templates.TemplateResponse(
        "index.html",
        {"request": request, "rows": rows, "active_filter": filter, "now": int(time.time())},
    )

@app.get("/ui/items/list", response_class=HTMLResponse)
async def ui_items_list(request: Request, filter: str = "all"):
    rows = _sorted_rows(None if filter == "all" else filter)
    return templates.TemplateResponse(
        "_item_list.html",
        {"request": request, "rows": rows, "active_filter": filter, "now": int(time.time())},
    )

@app.post("/ui/items/add", response_class=HTMLResponse)
async def ui_items_add(request: Request, name: str = Form(...)):
    name = name.strip()
    if not name:
        return Response(status_code=204)
    db_manager.update_item(name, 0)
    payload = {"op": "add", "item": name, "status": 0, "timestamp": int(time.time())}
    await mqtt_manager.publish_message(payload)
    # Return the new row partial so HTMX can append it.
    new_row = next(
        (r for r in db_manager.fetch_all_with_timestamps() if r[0] == name), None
    )
    if new_row is None:
        return Response(status_code=204)
    return templates.TemplateResponse(
        "_item_row.html",
        {"request": request, "row": new_row, "now": int(time.time())},
    )

@app.post("/ui/items/{name}/cycle", response_class=HTMLResponse)
async def ui_items_cycle(request: Request, name: str):
    current = db_manager.fetch_all().get(name)
    if current is None:
        return Response(status_code=404)
    new_status = (current + 1) % 3
    db_manager.update_item(name, new_status)
    payload = {"op": "update", "item": name, "status": new_status, "timestamp": int(time.time())}
    await mqtt_manager.publish_message(payload)
    updated_row = next(
        (r for r in db_manager.fetch_all_with_timestamps() if r[0] == name), None
    )
    return templates.TemplateResponse(
        "_item_row.html",
        {"request": request, "row": updated_row, "now": int(time.time())},
    )

@app.delete("/ui/items/{name}")
async def ui_items_delete(name: str):
    db_manager.delete_item(name)
    payload = {"op": "remove", "item": name, "status": None, "timestamp": int(time.time())}
    await mqtt_manager.publish_message(payload)
    return Response(status_code=200)
