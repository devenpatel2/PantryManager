import json

def split_into_chunks(items: dict[str, int], timestamp: int, max_size: int) -> list[dict]:
    """Split a bulk message into smaller chunks, ensuring each payload stays under the max size."""
    chunks = []
    current_chunk = {}
    base_payload = {"op": "bulk", "items": {}, "timestamp": timestamp}
    base_size = len(json.dumps(base_payload))  # Size of an empty bulk payload

    current_size = base_size

    for name, status in items.items():
        # Calculate the size of adding this item to the current chunk
        item_entry = json.dumps({name: status})[1:-1]  # Get just the key-value pair
        item_size = len(item_entry) + 2  # Add 2 for the `, ` separator in the JSON

        # If adding this item exceeds the max size, finalize the current chunk and start a new one
        if current_size + item_size > max_size:
            chunks.append({"op": "bulk", "items": current_chunk, "timestamp": timestamp})
            current_chunk = {}
            current_size = base_size

        # Add the item to the current chunk
        current_chunk[name] = status
        current_size += item_size

    # Add the last chunk
    if current_chunk:
        chunks.append({"op": "bulk", "items": current_chunk, "timestamp": timestamp})

    return chunks
