# app/database.py
import sqlite3
import time
from typing import Dict, List, Tuple

class DatabaseManager:
    def __init__(self, db_path: str):
        self.db_path = db_path

    def _now(self) -> int:
        return int(time.time())

    def get_all_items(self):
        """Retrieve all items from the database (name -> status)."""
        conn = sqlite3.connect(self.db_path)
        c = conn.cursor()
        c.execute("SELECT name, status FROM items")
        items = c.fetchall()
        conn.close()
        return {name: status for name, status in items}


    def initialize(self, initial_data: list[dict] = None):
        """Create / migrate the schema and seed initial data if empty."""
        conn = sqlite3.connect(self.db_path)
        c = conn.cursor()
        c.execute(
            """CREATE TABLE IF NOT EXISTS items (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                name TEXT,
                status INTEGER
            )"""
        )
        # Idempotent migration: add updated_at column if missing.
        try:
            c.execute("ALTER TABLE items ADD COLUMN updated_at INTEGER DEFAULT 0")
        except sqlite3.OperationalError:
            pass  # column already exists

        # One-time dedup: the previous update_item used INSERT OR REPLACE
        # without a UNIQUE constraint on name, so duplicate rows accumulated.
        # Keep the highest-id row per name.
        c.execute(
            """DELETE FROM items
               WHERE id NOT IN (SELECT MAX(id) FROM items GROUP BY name)"""
        )
        conn.commit()

        if initial_data:
            # Additive seed: insert any YAML item whose name isn't already in
            # the DB. Subsumes the "empty DB" case and lets the user add new
            # items by editing the YAML + restarting the server.
            c.execute("SELECT name FROM items")
            existing = {row[0] for row in c.fetchall()}
            now = self._now()
            for item in initial_data:
                if item["name"] not in existing:
                    c.execute(
                        "INSERT INTO items (name, status, updated_at) VALUES (?, ?, ?)",
                        (item["name"], item["status"], now),
                    )
            conn.commit()
        conn.close()

    def fetch_all(self) -> Dict[str, int]:
        """Retrieve all items as a dictionary (name -> status)."""
        conn = sqlite3.connect(self.db_path)
        c = conn.cursor()
        c.execute("SELECT name, status FROM items")
        items = {name: status for name, status in c.fetchall()}
        conn.close()
        return items

    def fetch_all_with_timestamps(self) -> List[Tuple[str, int, int]]:
        """Return (name, status, updated_at) tuples for every item."""
        conn = sqlite3.connect(self.db_path)
        c = conn.cursor()
        c.execute("SELECT name, status, COALESCE(updated_at, 0) FROM items")
        rows = c.fetchall()
        conn.close()
        return rows

    def update_item(self, name: str, status: int):
        """Insert or update a single item. Sets updated_at to now."""
        now = self._now()
        conn = sqlite3.connect(self.db_path)
        c = conn.cursor()
        c.execute(
            "UPDATE items SET status = ?, updated_at = ? WHERE name = ?",
            (status, now, name),
        )
        if c.rowcount == 0:
            c.execute(
                "INSERT INTO items (name, status, updated_at) VALUES (?, ?, ?)",
                (name, status, now),
            )
        conn.commit()
        conn.close()

    def delete_item(self, name: str):
        """Delete a single item by name."""
        conn = sqlite3.connect(self.db_path)
        c = conn.cursor()
        c.execute("DELETE FROM items WHERE name = ?", (name,))
        conn.commit()
        conn.close()
