#!/usr/bin/env python3
"""
mcp/kinpod_mcp/server.py
KinPod FastMCP Server

Enables any autonomous agent (such as OpenClaw, Claude, Cursor) to discover,
inhabit, and physically interact through KinPod wearable hardware.
"""

import sys
import logging
from typing import Optional
from mcp.server.fastmcp import FastMCP

from kinpod_mcp.client import KinPodClient

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger("KinPodMCP")

# Initialize FastMCP Server
mcp = FastMCP("kinpod", dependencies=["httpx", "pydantic"])
client = KinPodClient(base_url="http://127.0.0.1:8000")

@mcp.tool()
async def kinpod_status() -> str:
    """
    Check the connection and telemetry status of the physical KinPod wearable.
    Returns: Battery level, connection state, Wi-Fi signal, and active Kin avatar.
    """
    status = await client.get_status()
    if not status.get("connected"):
        return f"KinPod Offline or KinBridge unreachable: {status.get('error', 'Device not connected')}"
    return (
        f"KinPod Online [OK]\n"
        f"Active Kin Avatar: {status.get('active_kin', 'astro')}\n"
        f"Battery: {status.get('battery_mv', 3950)} mV\n"
        f"Wi-Fi RSSI: {status.get('wifi_rssi', -50)} dBm\n"
        f"Display Dimmed: {status.get('dimmed', False)}"
    )

@mcp.tool()
async def kinpod_inhabit(agent_id: str, avatar_id: str, title: Optional[str] = None) -> str:
    """
    Inhabit the physical KinPod wearable and change the character avatar on the AMOLED display.

    Parameters:
    - agent_id: Identifier of the calling agent (e.g. 'openclaw', 'claude', 'gemini')
    - avatar_id: The Kin character to display ('crab', 'astro', 'hermes', 'muse', 'octopus', 'dots')
    - title: Optional friendly name displayed on screen (e.g. 'OpenClaw Agent')
    """
    res = await client.inhabit(agent_id=agent_id, avatar_id=avatar_id, title=title)
    if res.get("status") == "ok":
        return f"Successfully inhabited KinPod. Avatar '{avatar_id}' is now active on AMOLED display."
    return f"Failed to inhabit KinPod: {res.get('message')}"

@mcp.tool()
async def kinpod_speak(text: str, voice: str = "onyx") -> str:
    """
    Make KinPod speak aloud to the user using the wearable's hardware speaker (ES8311).

    Parameters:
    - text: The speech message to synthesize and play aloud.
    - voice: Voice model profile ('onyx', 'nova', 'shimmer', 'echo', 'alloy').
    """
    res = await client.speak(text=text, voice=voice)
    if res.get("status") == "spoken":
        return f"Spoke message on KinPod: '{text}' ({res.get('bytes')} PCM bytes streamed)"
    return f"Error speaking on KinPod: {res.get('message')}"

@mcp.tool()
async def kinpod_request_approval(action: str, details: str, risk: str = "HIGH", timeout_sec: int = 60) -> str:
    """
    Request physical Human-In-The-Loop approval on the KinPod screen.
    Displays a touch modal on the user's chest with tactile Approve/Reject buttons.
    Execution pauses until the user taps the screen or timeout expires.

    Parameters:
    - action: Short action name (e.g. 'deploy_prod', 'rm_rf_build', 'transfer_funds')
    - details: Human explanation of what will be executed and why.
    - risk: Risk level ('LOW', 'MEDIUM', 'HIGH', 'CRITICAL')
    - timeout_sec: Seconds to wait for user touch response (default 60s).
    """
    import uuid
    action_id = f"act_{uuid.uuid4().hex[:6]}"
    res = await client.request_approval(
        action_id=action_id,
        agent="openclaw",
        action=action,
        details=details,
        risk=risk,
        timeout_sec=timeout_sec
    )
    if res.get("approved"):
        return f"[APPROVAL GRANTED]: Human approved '{action}' via KinPod touch screen."
    return f"[APPROVAL REJECTED]: Action '{action}' was rejected or timed out (reason: {res.get('reason', 'user_rejected')})."

def main():
    """Runs the FastMCP server over standard I/O (stdio)."""
    mcp.run()

if __name__ == "__main__":
    main()
