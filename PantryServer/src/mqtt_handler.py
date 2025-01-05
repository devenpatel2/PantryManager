import asyncio
import json
import time
from aiomqtt import Client
from .helper import split_into_chunks

MQTT_BROKER = "localhost"
MQTT_PORT = 1883
TOPIC_PUBLISH = "/pantry/items"
TOPIC_REQUEST = "/pantry/request"

class MQTTManager:
    def __init__(self, db_manager):
        self.client = Client(MQTT_BROKER, MQTT_PORT)
        self._db_manager = db_manager

    async def connect(self):
        """Connect to the MQTT broker."""
        await self.client.__aenter__()

    async def disconnect(self):
        """Disconnect from the MQTT broker."""
        await self.client.__aexit__(None, None, None)

    async def publish_message(self, payload: dict):
        """Publish a message to the MQTT topic."""
        await self.client.publish(TOPIC_PUBLISH, json.dumps(payload))
        await asyncio.sleep(0.1)

    async def publish_bulk(self):
        """Publish all items in the database in bulk."""
        items = self._db_manager.get_all_items()
        timestamp = int(time.time())
        chunks = split_into_chunks(items, timestamp, max_size=200)

        for chunk in chunks:
            await self.publish_message(chunk)

    async def subscribe_to_request(self):
        """Subscribe to the request topic and handle bulk requests."""
        await self.client.subscribe(TOPIC_REQUEST)
        async for message in self.client.messages:
            data = json.loads(message.payload.decode())
            if data.get("op") == "request":
                await self.publish_bulk()
