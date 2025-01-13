import asyncio
import time
import yaml
from fastapi import FastAPI, BackgroundTasks
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
    tasks.append(asyncio.create_task(mqtt_manager.subscribe_to_request()))
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
    background_tasks.add_task(mqtt_manager.publish, payload)
    return {"message": f"Item {data.op} operation successful"}
