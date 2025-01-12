import aiohttp
import asyncio
import logging
import time

logging.basicConfig(level=logging.INFO)
class WeatherHandler:
    def __init__(self, api_url, latitude, longitude, update_interval=10):
        """
        Initialize the WeatherHandler.

        :param api_url: The base URL for the weather API (e.g., Open-Meteo)
        :param latitude: Latitude of the location to fetch weather for
        :param longitude: Longitude of the location to fetch weather for
        :param update_interval: Interval for weather updates in seconds (default: 15 minutes)
        """
        self.api_url = api_url
        self.latitude = latitude
        self.longitude = longitude
        self.update_interval = update_interval
        self.cached_weather = None
        self.last_update_time = 0

    async def fetch_weather(self):
        """
        Fetch weather data from the API and update the cache.
        """
        try:
            params = {
                "latitude": self.latitude,
                "longitude": self.longitude,
                "current_weather": "True",
                "current": ["temperature_2m", "precipitation"],
                "hourly": ["temperature_2m", "precipitation"],
                "timezone": "auto"
            }
            async with aiohttp.ClientSession() as session:
                async with session.get(self.api_url, params=params) as response:
                    if response.status == 200:
                        data = await response.json()
                        self.cached_weather = self.parse_weather_data(data)
                        self.last_update_time = int(time.time())
                        logging.debug("Weather data updated successfully.")
                    else:
                        logging.error(f"Failed to fetch weather data. HTTP Status: {response.status}")
        except Exception as e:
            logging.error(f"Error fetching weather data: {e}", exc_info=True)

    def parse_weather_data(self, data):
        """
        Parse the weather data from the API response.

        :param data: The raw API response
        :return: Parsed weather data as a dictionary
        """
        try:
            current_weather = data.get("current_weather", {})
            current_weather_units = data.get("current_weather_units", {})
            hourly = data.get("hourly", {})

            weather = {
                "temperature": current_weather.get("temperature"),
                "precipitation": hourly.get("precipitation", [None])[0],
                "warning": None,  # Placeholder for future severe weather warnings
                "timestamp": self.last_update_time
            }

            return weather
        except Exception as e:
            logging.error(f"Error parsing weather data: {e}")
            return None

    def get_cached_weather(self):
        """
        Get the most recently cached weather data.

        :return: Cached weather data as a dictionary
        """
        return self.cached_weather

    async def start_periodic_updates(self):
        """
        Start periodic weather updates.
        """
        while True:
            await self.fetch_weather()
            await asyncio.sleep(self.update_interval)

# Example usage (to be integrated with main.py):
if __name__ == "__main__":

    # Initialize WeatherHandler with Open-Meteo API and coordinates for a location
    weather_handler = WeatherHandler(
        api_url="https://api.open-meteo.com/v1/forecast",
        latitude=48.1147,  # Munich - 81377 
        longitude=11.476975
    )

    async def main():
        # Start periodic updates
        await weather_handler.start_periodic_updates()
    asyncio.run(main())

