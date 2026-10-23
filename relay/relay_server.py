#!/usr/bin/env python3
"""
relay/relay_server.py
KinPod Cloud Rendezvous Pairing Server (Standalone Microservice)

Provides single-use / short-lived 4-digit PIN matching for connecting
autonomous agents (OpenClaw) with KinPod wearables over cellular/WAN.
"""

import time
import random
import logging
from typing import Dict, Any, Optional
from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger("KinRelay")

app = FastAPI(
    title="KinPod Pairing Relay Server",
    description="4-Digit Rendezvous Broker for Wearable & Autonomous Agent Pairing",
    version="1.0.0"
)

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# In-memory storage: pin -> {"url": str, "expires_at": float}
active_pairs: Dict[str, Dict[str, Any]] = {}

class PairRequest(BaseModel):
    url: str
    ttl_sec: Optional[int] = 600

def cleanup_expired():
    now = time.time()
    for pin in list(active_pairs.keys()):
        if active_pairs[pin]["expires_at"] < now:
            del active_pairs[pin]

@app.get("/")
def index():
    return {
        "service": "KinPod Pairing Relay",
        "active_pairs_count": len(active_pairs)
    }

@app.post("/api/pair")
def register_pair(req: PairRequest):
    """
    Registers an agent's roaming WebSocket tunnel and assigns a 4-digit PIN (e.g. 4821).
    Valid for ttl_sec (default: 600s / 10 minutes).
    """
    cleanup_expired()
    now = time.time()
    ttl = req.ttl_sec or 600

    # Generate unique 4-digit PIN
    for _ in range(100):
        pin = f"{random.randint(1000, 9999)}"
        if pin not in active_pairs:
            active_pairs[pin] = {
                "url": req.url,
                "expires_at": now + ttl
            }
            logger.info(f"Registered Pairing PIN: {pin} -> {req.url} (TTL: {ttl}s)")
            return {
                "status": "ok",
                "pin": pin,
                "url": req.url,
                "expires_in": ttl
            }

    raise HTTPException(status_code=503, detail="Pairing pool exhausted. Please retry shortly.")

@app.get("/api/pair/{pin}")
def resolve_pair(pin: str):
    """
    Resolves the 4-digit PIN entered on the KinPod screen to its target WebSocket tunnel.
    """
    cleanup_expired()
    now = time.time()
    item = active_pairs.get(pin)
    if not item or item["expires_at"] < now:
        raise HTTPException(status_code=404, detail="PIN not found or expired")

    logger.info(f"Resolved PIN: {pin} -> {item['url']}")
    return {
        "status": "ok",
        "pin": pin,
        "url": item["url"],
        "expires_in": int(item["expires_at"] - now)
    }

if __name__ == "__main__":
    import uvicorn
    uvicorn.run("relay_server:app", host="0.0.0.0", port=8001, reload=True)
