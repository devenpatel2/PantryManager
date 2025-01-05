import asyncio
import json
import sqlite3
from aiomqtt import Client
from fastapi import FastAPI, BackgroundTasks
from pydantic import BaseModel
import yaml

app = FastAPI()

# MQTT Configuration
MQTT_BROKER = "localhost"
MQTT_PORT = 1883
TOPIC_PUBLISH = "/pantry/items"
DATABASE = "items.db"

# MQTT Client
mqtt_client = Client(MQTT_BROKER, MQTT_PORT)

# Item model
class Item(BaseModel):
    name: str
    status: int  # 0 = In Stock, 1 = Out of Stock, 2 = Surplus


# SQLite Helper Functions
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
    items_list = [{"name": name, "status": status} for name, status in items]

    # Sort the items:
    # 1. Out-of-stock (status == 1) first
    # 2. Alphabetically by name within each group
    sorted_items = sorted(items_list, key=lambda x: (x["status"] != 1, x["name"].lower()))
    return items_list


# MQTT Publishing
async def publish_items():
    """Publish the list of items to the MQTT topic."""
    items = await get_all_items()
    payload = {"items": items}
    await mqtt_client.publish(TOPIC_PUBLISH, json.dumps(payload), retain=True)
    await asyncio.sleep(1)


@app.on_event("startup")
async def on_startup():
    """Startup tasks: Initialize DB and connect to MQTT."""
    await init_db()
    async with mqtt_client:
        await publish_items()


#@app.on_event("shutdown")
#async def on_shutdown():
#    """Shutdown tasks: Disconnect MQTT."""
#    await mqtt_client.disconnect()


@app.get("/items")
async def get_items():
    """API endpoint to retrieve all items."""
    return {"items": await get_all_items()}


@app.post("/items")
async def add_item(item: Item, background_tasks: BackgroundTasks):
    """Add an item to the database and publish updated list."""
    conn = sqlite3.connect(DATABASE)
    c = conn.cursor()
    c.execute("INSERT INTO items (name, status) VALUES (?, ?)", (item.name, item.status))
    conn.commit()
    conn.close()

    # Publish updated list in the background
    background_tasks.add_task(publish_items)
    return {"message": "Item added successfully"}
