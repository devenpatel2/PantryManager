import asyncio
from fastapi import FastAPI, BackgroundTasks
from .mqtt_handler import MQTTManager
from .database import DatabaseManager
from .models import SingleItemMessage, BulkMessage
import time
import yaml

app = FastAPI()

# Configuration
DB_PATH = "src/items.db"

# Managers
db_manager = DatabaseManager(DB_PATH)
mqtt_manager = MQTTManager(db_manager)

@app.on_event("startup")
async def startup():
    # Initialize database and MQTT
    with open("src/initial_items.yaml", "r") as f:
        initial_data = yaml.safe_load(f)["items"]
    db_manager.initialize(initial_data)
    await mqtt_manager.connect()
    await mqtt_manager.publish_bulk()
    asyncio.create_task(mqtt_manager.subscribe_to_request())

@app.on_event("shutdown")
async def shutdown():
    await mqtt_manager.disconnect()

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
