# app/database.py
import sqlite3
from typing import Dict

class DatabaseManager:
    def __init__(self, db_path: str):
        self.db_path = db_path

    def init_db():
        """Initialize the database and populate with initial data."""
        conn = sqlite3.connect(self.db_path)
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

    def get_all_items(self):
        """Retrieve all items from the database."""
        conn = sqlite3.connect(self.db_path)
        c = conn.cursor()
        c.execute("SELECT name, status FROM items")
        items = c.fetchall()
        conn.close()
        return {name: status for name, status in items}


    def initialize(self, initial_data: list[dict] = None):
        """Initialize the database and populate it with data if empty."""
        conn = sqlite3.connect(self.db_path)
        c = conn.cursor()
        c.execute(
            """CREATE TABLE IF NOT EXISTS items (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                name TEXT,
                status INTEGER
            )"""
        )
        conn.commit()
        if initial_data:
            c.execute("SELECT COUNT(*) FROM items")
            if c.fetchone()[0] == 0:
                for item in initial_data:
                    c.execute(
                        "INSERT INTO items (name, status) VALUES (?, ?)",
                        (item["name"], item["status"]),
                    )
                conn.commit()
        conn.close()

    def fetch_all(self) -> Dict[str, int]:
        """Retrieve all items as a dictionary."""
        conn = sqlite3.connect(self.db_path)
        c = conn.cursor()
        c.execute("SELECT name, status FROM items")
        items = {name: status for name, status in c.fetchall()}
        conn.close()
        return items

    def update_item(self, name: str, status: int):
        """Add or update a single item."""
        conn = sqlite3.connect(self.db_path)
        c = conn.cursor()
        c.execute("INSERT OR REPLACE INTO items (name, status) VALUES (?, ?)", (name, status))
        conn.commit()
        conn.close()

    def delete_item(self, name: str):
        """Delete a single item."""
        conn = sqlite3.connect(self.db_path)
        c = conn.cursor()
        c.execute("DELETE FROM items WHERE name = ?", (name,))
        conn.commit()
        conn.close()
