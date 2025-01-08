import asyncio
import json
import sqlite3
from aiomqtt import Client
from fastapi import FastAPI, BackgroundTasks
from pydantic import BaseModel
from typing import Optional
import time
import yaml

import helper

app = FastAPI()

# MQTT Configuration
MQTT_BROKER = "localhost"
MQTT_PORT = 1883
TOPIC_PUBLISH = "/pantry/items"
TOPIC_REQUEST = "/pantry/request"
DATABASE = "items.db"

# MQTT Client
mqtt_client = Client(MQTT_BROKER, MQTT_PORT)
task = None

# Data Models
class BulkMessage(BaseModel):
    op: str  # Must be "bulk"
    items: dict[str, int]
    timestamp: int

class SingleItemMessage(BaseModel):
    item: str
    status: Optional[int]  # null for removal
    op: str  # "add", "update", or "remove"
    timestamp: int

async def init_db():
    """Initialize the database and populate with initial data."""
    conn = sqlite3.connect(DATABASE)
    c = conn.cursor()
    c.execute(
        """CREATE TABLE IF NOT EXISTS items (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT,
            status INTEGER
        )"""
    )
    conn.commit()
    
    # Populate database if empty
    c.execute("SELECT COUNT(*) FROM items")
    count = c.fetchone()[0]
    if count == 0:
        # Try loading from YAML, fallback to hardcoded data
        try:
            with open("initial_items.yaml", "r") as f:
                items = yaml.safe_load(f)["items"]
        except FileNotFoundError:
            # Hardcoded fallback
            items = [
                {"name": "Milk", "status": 1},
                {"name": "Bread", "status": 1},
                {"name": "Cheese", "status": 1},
                {"name": "Juice", "status": 1},
                {"name": "Sugar", "status": 0},
                {"name": "Salt", "status": 0},
                {"name": "Coffee", "status": 2},
                {"name": "Pepper", "status": 0},
                {"name": "Herbs", "status": 0},
                {"name": "Coriander", "status": 2},
            ]
        
        # Insert items into the database
        for item in items:
            c.execute("INSERT INTO items (name, status) VALUES (?, ?)", (item["name"], item["status"]))
        conn.commit()
    conn.close()

async def get_all_items():
    """Retrieve all items from the database."""
    conn = sqlite3.connect(DATABASE)
    c = conn.cursor()
    c.execute("SELECT name, status FROM items")
    items = c.fetchall()
    conn.close()
    items_dict = {name: status for name, status in items}
    return items_dict

async def publish_message(payload: dict):
    """Publish a message to the MQTT topic."""
    await mqtt_client.publish(TOPIC_PUBLISH, json.dumps(payload))
    await asyncio.sleep(1)

async def publish_bulk():
    # Initial bulk publish
    items = await get_all_items()
    timestamp = int(time.time())
    chunks = helper.split_into_chunks(items, timestamp, 220)
    # Publish the bulk update to MQTT
    for chunk in chunks:
        await publish_message(chunk)


async def mqtt_listener_callback(client):
    
   #  async with client.messages() as messages:
    await client.subscribe(TOPIC_REQUEST)
    async for message in client.messages:
        data = json.loads(message.payload.decode())
        if data.get("op") == "request":
            await publish_bulk()


# FastAPI Routes
@app.on_event("startup")
async def on_startup():
    """Startup tasks: Initialize DB and connect to MQTT."""
    await init_db()
    await mqtt_client.__aenter__()
    await publish_bulk()
    #loop = asyncio.get_event_loop()
    global task
    task = asyncio.create_task(mqtt_listener_callback(mqtt_client))

@app.on_event("shutdown")
async def shutdown_event():
    if task:
        task.cancel()
    await mqtt_client.__aexit__(None, None, None)


@app.post("/items")
async def handle_item(data: SingleItemMessage, background_tasks: BackgroundTasks):
    """Handle add/update/remove operations for a single item."""
    conn = sqlite3.connect(DATABASE)
    c = conn.cursor()

    if data.op == "add" or data.op == "update":
        c.execute("INSERT OR REPLACE INTO items (name, status) VALUES (?, ?)", (data.item, data.status))
    elif data.op == "remove":
        c.execute("DELETE FROM items WHERE name = ?", (data.item,))
   
    conn.commit()
    conn.close()

    # Publish the update to MQTT
    payload = {
        "item": data.item,
        "status": data.status,
        "op": data.op,
        "timestamp": data.timestamp
    }
    background_tasks.add_task(publish_message, payload)

    return {"message": f"Item {data.op} operation successful"}

@app.post("/bulk")
async def handle_bulk(data: BulkMessage, background_tasks: BackgroundTasks):
    """Handle bulk updates."""
    conn = sqlite3.connect(DATABASE)
    c = conn.cursor()

    # Replace all items in the database
    c.execute("DELETE FROM items")
    for name, status in data.items.items():
        c.execute("INSERT INTO items (name, status) VALUES (?, ?)", (name, status))
    
    conn.commit()
    conn.close()

    # Split the items dictionary into chunks if needed
    items = data.items
    timestamp = data.timestamp
    chunks = helper.split_into_chunks(items, timestamp, 50)
    # Publish the bulk update to MQTT
    for chunk in chunks:
        background_tasks.add_task(publish_message, chunk)


    return {"message": "Bulk update successful"}
