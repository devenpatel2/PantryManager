import asyncio
import json
import time
from aiomqtt import Client, MqttError
from .helper import split_into_chunks

MQTT_BROKER = "localhost"
MQTT_PORT = 1883
TOPIC_ITEMS = "/pantry/items"
TOPIC_REQUEST = "/pantry/request"
TOPIC_WEATHER = "/pantry/weather"
TOPIC_UPDATE = "/pantry/update"

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

    async def publish_weather(self, weather_data):
        """Publish weather data to the weather topic."""
        payload = {"op": "weather", "data": weather_data, "timestamp": int(time.time())}
        await self.client.publish(TOPIC_WEATHER, json.dumps(payload))

    async def publish_message(self, payload: dict):
        """Publish a message to the MQTT topic."""
        await self.client.publish(TOPIC_ITEMS, json.dumps(payload))
        await asyncio.sleep(0.1)

    async def publish_bulk(self):
        """Publish all items in the database in bulk."""
        items = self._db_manager.get_all_items()
        timestamp = int(time.time())
        chunks = split_into_chunks(items, timestamp, max_size=200)

        for chunk in chunks:
            await self.publish_message(chunk)

    async def subscribe_to_topics(self):
        """Subscribe to client-originated topics and dispatch by topic."""
        await self.client.subscribe(TOPIC_REQUEST)
        await self.client.subscribe(TOPIC_UPDATE)
        try:
            async for message in self.client.messages:
                topic = str(message.topic)
                data = json.loads(message.payload.decode())
                if topic == TOPIC_REQUEST:
                    if data.get("op") == "request":
                        await self.publish_bulk()
                elif topic == TOPIC_UPDATE:
                    await self._handle_item_update(data)
        except MqttError:
            pass

    async def _handle_item_update(self, data):
        """Persist a client-originated item update and rebroadcast on the items topic."""
        if data.get("op") != "update":
            return
        item = data.get("item")
        status = data.get("status")
        if item is None or status is None:
            return
        self._db_manager.update_item(item, status)
        await self.publish_message({
            "op": "update",
            "item": item,
            "status": status,
            "timestamp": int(time.time()),
        })

