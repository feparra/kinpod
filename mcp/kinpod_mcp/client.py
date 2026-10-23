"""
mcp/kinpod_mcp/client.py
HTTP/WebSocket Client bridge for KinPod Hardware.
Communicates with KinBridge server (http://127.0.0.1:8000).
"""

import logging
from typing import Dict, Any, Optional
import httpx

logger = logging.getLogger("KinPodClient")

class KinPodClient:
    def __init__(self, base_url: str = "http://127.0.0.1:8000"):
        self.base_url = base_url.rstrip("/")

    async def get_status(self) -> Dict[str, Any]:
        """Fetch real-time hardware status, battery, active avatar, and connection."""
        async with httpx.AsyncClient(timeout=5.0) as client:
            try:
                resp = await client.get(f"{self.base_url}/api/status")
                if resp.status_code == 200:
                    return resp.json()
                return {"connected": False, "error": f"HTTP {resp.status_code}"}
            except Exception as e:
                return {"connected": False, "error": str(e)}

    async def inhabit(self, agent_id: str, avatar_id: str, title: Optional[str] = None) -> Dict[str, Any]:
        """Set the active visual Kin on the AMOLED display."""
        payload = {
            "agent_id": agent_id,
            "avatar_id": avatar_id,
            "custom_title": title
        }
        async with httpx.AsyncClient(timeout=8.0) as client:
            resp = await client.post(f"{self.base_url}/api/inhabit", json=payload)
            if resp.status_code == 200:
                return resp.json()
            return {"status": "error", "message": resp.text}

    async def speak(self, text: str, voice: str = "onyx", kin_id: Optional[str] = None) -> Dict[str, Any]:
        """Synthesize and speak text on the wearer's chest speaker."""
        payload = {
            "text": text,
            "voice": voice,
            "kin": kin_id
        }
        async with httpx.AsyncClient(timeout=30.0) as client:
            resp = await client.post(f"{self.base_url}/api/speak", json=payload)
            if resp.status_code == 200:
                return resp.json()
            return {"status": "error", "message": resp.text}

    async def request_approval(
        self,
        action_id: str,
        agent: str,
        action: str,
        details: str,
        risk: str = "HIGH",
        timeout_sec: int = 60
    ) -> Dict[str, Any]:
        """
        Request physical Human-In-The-Loop approval on KinPod AMOLED screen.
        Blocks until the human taps Approve/Reject or timeout expires.
        """
        payload = {
            "action_id": action_id,
            "agent": agent,
            "action": action,
            "details": details,
            "risk": risk,
            "timeout_sec": timeout_sec
        }
        async with httpx.AsyncClient(timeout=float(timeout_sec + 5)) as client:
            resp = await client.post(f"{self.base_url}/api/approval/request", json=payload)
            if resp.status_code == 200:
                return resp.json()
            return {"action_id": action_id, "approved": False, "reason": resp.text}

    async def get_tunnel(self) -> Dict[str, Any]:
        """Fetch or start Cloudflare public tunnel for remote outdoor roaming."""
        async with httpx.AsyncClient(timeout=30.0) as client:
            try:
                resp = await client.get(f"{self.base_url}/api/tunnel")
                if resp.status_code == 200:
                    return resp.json()
                return {"status": "error", "message": f"HTTP {resp.status_code}"}
            except Exception as e:
                return {"status": "error", "message": str(e)}
