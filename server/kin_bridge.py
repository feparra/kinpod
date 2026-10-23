#!/usr/bin/env python3
"""
server/kin_bridge.py
KinBridge WebSocket & REST Hub (v6.0 Final)

Coordinates bidirectional binary streaming between:
- Waveshare ESP32-S3 AMOLED 2.06" (via WebSocket /ws/kinpod)
- Universal Agent Router (Gemini, OpenClaw, Hermes, Claude, Ollama)
- KinPod MCP Server (for autonomous agents like OpenClaw)
- Telegram Mirror Notification Channel
"""

import io
import os
import json
import wave
import asyncio
import logging
from typing import Dict, Any, Optional

import httpx
from fastapi import FastAPI, WebSocket, WebSocketDisconnect, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel

from kin_router import process_kin_interaction, THE_KIN_ROSTER

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger("KinBridge")

app = FastAPI(
    title="KinBridge Server",
    description="Bidirectional WebSocket & Agent Gateway for KinPod Wearable",
    version="6.0.0",
)

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

TG_BOT_TOKEN = os.getenv("TG_BOT_TOKEN", "")
TG_CHAT_ID = os.getenv("TG_CHAT_ID", "")

# Active connection state
active_kinpod_ws: Optional[WebSocket] = None
current_device_status = {
    "connected": False,
    "active_kin": "astro",
    "battery_mv": 3950,
    "wifi_rssi": -52,
    "dimmed": False,
    "last_seen": None,
}

# Pending approvals for Human-In-The-Loop
pending_approvals: Dict[str, asyncio.Future] = {}

class ApprovalRequest(BaseModel):
    action_id: str
    agent: str
    action: str
    details: str
    risk: str = "MEDIUM"
    timeout_sec: int = 60

class SpeakRequest(BaseModel):
    text: str
    kin: Optional[str] = None
    voice: Optional[str] = "onyx"

class InhabitRequest(BaseModel):
    agent_id: str
    avatar_id: str
    custom_title: Optional[str] = None

def pcm16_to_wav(pcm_data: bytes) -> bytes:
    """Encapsulates raw 16kHz 16-bit Mono PCM bytes into a standard RIFF WAV buffer."""
    buf = io.BytesIO()
    with wave.open(buf, "wb") as wf:
        wf.setnchannels(1)
        wf.setsampwidth(2)
        wf.setframerate(16000)
        wf.writeframes(pcm_data)
    return buf.getvalue()

async def mirror_telegram(wav_bytes: bytes, transcript: str, reply: str, kin_id: str):
    """Sends audio recording and markdown transcript to Telegram channel."""
    if not TG_BOT_TOKEN or not TG_CHAT_ID:
        return
    url = f"https://api.telegram.org/bot{TG_BOT_TOKEN}"
    try:
        async with httpx.AsyncClient(timeout=15.0) as client:
            await client.post(
                f"{url}/sendAudio",
                data={"chat_id": TG_CHAT_ID, "caption": f"🎙️ [{kin_id.upper()}]: {transcript}"},
                files={"audio": ("voice.wav", wav_bytes, "audio/wav")},
            )
            await client.post(
                f"{url}/sendMessage",
                json={
                    "chat_id": TG_CHAT_ID,
                    "text": f"✨ *{kin_id.upper()}:*\n{reply}",
                    "parse_mode": "Markdown",
                },
            )
    except Exception as e:
        logger.warning(f"Telegram mirror warning: {e}")

# -----------------------------------------------------------------------------
# REST API (For MCP Server & External Agents like OpenClaw)
# -----------------------------------------------------------------------------
@app.get("/api/status")
async def get_status():
    """Returns current KinPod device connection and telemetry."""
    return current_device_status

@app.post("/api/inhabit")
async def inhabit_device(req: InhabitRequest):
    """Allows an agent (e.g. OpenClaw) to inhabit KinPod and set its character avatar."""
    global active_kinpod_ws
    if req.avatar_id not in THE_KIN_ROSTER:
        raise HTTPException(status_code=400, detail=f"Avatar '{req.avatar_id}' not found in roster.")

    current_device_status["active_kin"] = req.avatar_id
    if active_kinpod_ws:
        await active_kinpod_ws.send_text(json.dumps({
            "event": "kin_switch",
            "kin": req.avatar_id,
            "agent": req.agent_id,
            "title": req.custom_title or THE_KIN_ROSTER[req.avatar_id]["name"]
        }))
    logger.info(f"Agent '{req.agent_id}' inhabited KinPod as avatar '{req.avatar_id}'")
    return {"status": "ok", "active_kin": req.avatar_id}

@app.post("/api/speak")
async def trigger_speak(req: SpeakRequest):
    """Makes KinPod speak text via hardware speaker ES8311."""
    global active_kinpod_ws
    if not active_kinpod_ws:
        raise HTTPException(status_code=503, detail="KinPod hardware is not connected.")

    kin_id = req.kin or current_device_status["active_kin"]
    from kin_router import synthesize_pcm16k
    pcm16k = await synthesize_pcm16k(req.text, req.voice or "onyx")

    await active_kinpod_ws.send_text(json.dumps({
        "event": "reply",
        "kin": kin_id,
        "text": req.text
    }))
    for i in range(0, len(pcm16k), 2048):
        await active_kinpod_ws.send_bytes(pcm16k[i : i + 2048])
        await asyncio.sleep(0.02)
    await active_kinpod_ws.send_text(json.dumps({"event": "tts_end"}))
    return {"status": "spoken", "bytes": len(pcm16k)}

@app.post("/api/approval/request")
async def request_human_approval(req: ApprovalRequest):
    """
    Pushes an approval modal to the wearable on the wearer's chest.
    Blocks until human taps Approve/Reject or timeout expires.
    """
    global active_kinpod_ws
    if not active_kinpod_ws:
        raise HTTPException(status_code=503, detail="KinPod hardware is not connected.")

    loop = asyncio.get_running_loop()
    future = loop.create_future()
    pending_approvals[req.action_id] = future

    await active_kinpod_ws.send_text(json.dumps({
        "event": "approval_request",
        "action_id": req.action_id,
        "agent": req.agent,
        "action": req.action,
        "details": req.details,
        "risk": req.risk,
    }))

    try:
        result = await asyncio.wait_for(future, timeout=float(req.timeout_sec))
        return result
    except asyncio.TimeoutError:
        pending_approvals.pop(req.action_id, None)
        return {"action_id": req.action_id, "approved": False, "reason": "timeout"}

s_tunnel_url: Optional[str] = None
s_tunnel_proc = None

def start_cloudflare_tunnel(port: int = 8000) -> Dict[str, Any]:
    global s_tunnel_url, s_tunnel_proc
    if s_tunnel_url:
        return {"status": "active", "url": s_tunnel_url, "http_url": s_tunnel_proc.tunnel if s_tunnel_proc else None}
    try:
        import pycloudflared
        s_tunnel_proc = pycloudflared.try_cloudflare(port=port)
        s_tunnel_url = s_tunnel_proc.tunnel.replace("https://", "wss://") + "/ws/kinpod"
        logger.info(f"🚀 Cloudflare Public Tunnel Active! KinPod Roaming URL: {s_tunnel_url}")
        return {"status": "active", "url": s_tunnel_url, "http_url": s_tunnel_proc.tunnel}
    except Exception as e:
        logger.error(f"Tunnel creation failed: {e}")
        return {"status": "error", "message": str(e)}

@app.on_event("startup")
async def on_startup():
    logger.info("KinBridge Hub initializing...")
    if os.getenv("KIN_AUTO_TUNNEL", "1") == "1":
        asyncio.get_event_loop().run_in_executor(None, start_cloudflare_tunnel, 8000)

@app.get("/api/tunnel")
def get_or_start_tunnel(port: int = 8000):
    """Starts or returns the active Cloudflare public tunnel for roaming outdoors."""
    return start_cloudflare_tunnel(port=port)

# -----------------------------------------------------------------------------
# PAIRING & RENDEZVOUS API (For 4-Digit PIN Wearable Linking)
# -----------------------------------------------------------------------------
s_active_pairs: Dict[str, Dict[str, Any]] = {}

class PairRequest(BaseModel):
    url: str
    ttl_sec: Optional[int] = 600

@app.post("/api/pair")
def register_pair(req: PairRequest):
    """Registers an agent's roaming WebSocket tunnel and assigns a 4-digit PIN."""
    import time, random
    now = time.time()
    for p in list(s_active_pairs.keys()):
        if s_active_pairs[p]["expires_at"] < now:
            del s_active_pairs[p]
    ttl = req.ttl_sec or 600
    for _ in range(100):
        pin = f"{random.randint(1000, 9999)}"
        if pin not in s_active_pairs:
            s_active_pairs[pin] = {"url": req.url, "expires_at": now + ttl}
            logger.info(f"Registered Pairing PIN {pin} -> {req.url}")
            return {"status": "ok", "pin": pin, "url": req.url, "expires_in": ttl}
    raise HTTPException(status_code=503, detail="Pairing pool full")

@app.get("/api/pair/{pin}")
def resolve_pair(pin: str):
    """Resolves a 4-digit PIN to its roaming WebSocket tunnel URL."""
    import time
    now = time.time()
    item = s_active_pairs.get(pin)
    if not item or item["expires_at"] < now:
        raise HTTPException(status_code=404, detail="PIN not found or expired")
    return {"status": "ok", "pin": pin, "url": item["url"]}


# -----------------------------------------------------------------------------
# WEBSOCKET ENDPOINT (ESP32-S3 Hardware / Simulator)
# -----------------------------------------------------------------------------
@app.websocket("/ws/kinpod")
async def kinpod_ws(ws: WebSocket):
    """Main bidirectional streaming WebSocket connection with KinPod hardware."""
    global active_kinpod_ws
    await ws.accept()
    if active_kinpod_ws and active_kinpod_ws != ws:
        try:
            await active_kinpod_ws.close()
        except Exception:
            pass
    active_kinpod_ws = ws
    current_device_status["connected"] = True
    logger.info("KinPod ESP32-S3 device connected successfully.")

    pcm_buf = bytearray()
    kin_id = current_device_status["active_kin"]

    try:
        while True:
            msg = await ws.receive()

            # Text control frames (JSON)
            if "text" in msg and msg["text"]:
                data = json.loads(msg["text"])
                evt = data.get("event")

                if evt == "ptt_start":
                    pcm_buf.clear()
                    kin_id = data.get("kin", current_device_status["active_kin"])
                    logger.info(f"PTT Start for Kin: {kin_id}")

                elif evt == "ptt_stop":
                    # Short Tap Protection (< 300ms / 9600 bytes at 16kHz 16-bit mono)
                    if len(pcm_buf) < 9600:
                        logger.warning(f"Short tap rejected: {len(pcm_buf)} bytes (< 9600)")
                        await ws.send_text(json.dumps({"event": "error", "message": "Tap corto"}))
                        continue

                    wav = pcm16_to_wav(bytes(pcm_buf))
                    trans, reply, pcm16k = await process_kin_interaction(wav, kin_id)

                    await ws.send_text(json.dumps({
                        "event": "reply",
                        "kin": kin_id,
                        "text": reply,
                        "transcript": trans
                    }))

                    # Stream synthesized audio chunks (2048 bytes every ~30ms)
                    for i in range(0, len(pcm16k), 2048):
                        await ws.send_bytes(pcm16k[i : i + 2048])
                        await asyncio.sleep(0.03)

                    await ws.send_text(json.dumps({"event": "tts_end"}))
                    asyncio.create_task(mirror_telegram(wav, trans, reply, kin_id))

                elif evt == "kin_switch":
                    kin_id = data.get("kin", "astro")
                    current_device_status["active_kin"] = kin_id
                    logger.info(f"Kin switched to: {kin_id}")

                elif evt == "approval_response":
                    action_id = data.get("action_id")
                    approved = data.get("approved", False)
                    if action_id in pending_approvals:
                        pending_approvals[action_id].set_result({
                            "action_id": action_id,
                            "approved": approved,
                            "actor": "kinpod_touch"
                        })
                        pending_approvals.pop(action_id, None)

                elif evt == "telemetry":
                    current_device_status["battery_mv"] = data.get("battery_mv", 3950)
                    current_device_status["wifi_rssi"] = data.get("rssi", -50)
                    current_device_status["dimmed"] = data.get("dimmed", False)

            # Audio binary frames (PCM from ES7210 microphone)
            elif "bytes" in msg and msg["bytes"]:
                pcm_buf.extend(msg["bytes"])

    except WebSocketDisconnect:
        logger.warning("KinPod ESP32-S3 disconnected.")
    except Exception as e:
        logger.warning(f"KinPod WebSocket exception: {e}")
    finally:
        if active_kinpod_ws == ws:
            active_kinpod_ws = None
            current_device_status["connected"] = False

if __name__ == "__main__":
    import uvicorn
    port = int(os.getenv("PORT", os.getenv("KINPOD_PORT", "8000")))
    uvicorn.run("kin_bridge:app", host="0.0.0.0", port=port, reload=True)
