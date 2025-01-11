from typing import Optional
from pydantic import BaseModel

class BulkMessage(BaseModel):
    op: str  # Must be "bulk"
    items: dict[str, int]
    timestamp: int

class SingleItemMessage(BaseModel):
    item: str
    status: Optional[int]  # null for removal
    op: str  # "add", "update", or "remove"
    timestamp: int

