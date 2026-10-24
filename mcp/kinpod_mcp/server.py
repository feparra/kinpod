#!/usr/bin/env python3
"""
mcp/kinpod_mcp/server.py
KinPod FastMCP Server

Enables any autonomous agent (such as OpenClaw, Claude, Cursor) to discover,
inhabit, and physically interact through KinPod wearable hardware.
"""

import sys
import os
import logging
from typing import Optional
from mcp.server.fastmcp import FastMCP

from kinpod_mcp.client import KinPodClient

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger("KinPodMCP")

# Initialize FastMCP Server
mcp = FastMCP("kinpod", dependencies=["httpx", "pydantic", "pycloudflared"])
client = KinPodClient(base_url=os.getenv("KINPOD_HUB_URL", "https://agent.thekinpod.com"))

@mcp.tool()
async def kinpod_expose_tunnel(port: int = 8000) -> str:
    """
    Automatically creates a zero-config, secure Cloudflare public tunnel for KinBridge.
    Enables KinPod wearable to communicate with your agent while roaming outdoors on a cellular hotspot.
    Returns: The public wss:// URL for the wearable.
    """
    try:
        # Check if KinBridge already has an active tunnel
        res = await client.get_tunnel()
        if res.get("status") == "active" and res.get("url"):
            tunnel_ws = res["url"]
            return (
                f"[KinPod Public Tunnel Active]\n"
                f"Public WebSocket URL: {tunnel_ws}\n\n"
                f"Copy '{tunnel_ws}' into KinPod setup portal ('KinBridge Server URL') to roam anywhere on 4G/5G!"
            )

        # Fallback to direct pycloudflared invocation
        import pycloudflared
        urls = pycloudflared.try_cloudflare(port=port)
        tunnel_ws = urls.tunnel.replace("https://", "wss://") + "/ws/kinpod"
        return (
            f"[KinPod Public Tunnel Active]\n"
            f"Public WebSocket URL: {tunnel_ws}\n"
            f"Public Web URL: {urls.tunnel}\n\n"
            f"Copy '{tunnel_ws}' into KinPod setup portal ('KinBridge Server URL') to roam anywhere on 4G/5G!"
        )
    except Exception as e:
        return f"Error starting Cloudflare tunnel: {e}"

DEFAULT_RELAY_URL = "https://relay.thekinpod.com"

@mcp.tool()
async def kinpod_generate_pairing_code(relay_url: Optional[str] = None) -> str:
    """
    Generates a temporary 4-digit pairing PIN (e.g. 4821) for the user to link their KinPod wearable.
    Zero app download, zero cables, zero manual typing of long URLs.
    The agent registers its Cloudflare roaming tunnel with the Cloud Relay, and the user enters
    the 4 digits on the KinPod AMOLED touchscreen to connect immediately!
    """
    import httpx

    # 1. Fetch active Cloudflare roaming tunnel URL
    res = await client.get_tunnel()
    if res.get("status") != "active" or not res.get("url"):
        return "[Error]: Cloudflare roaming tunnel is not active on KinBridge. Run kinpod_expose_tunnel first."

    tunnel_url = res["url"]
    target_relay = relay_url or DEFAULT_RELAY_URL

    # 2. Register with Relay Server
    try:
        async with httpx.AsyncClient(timeout=10.0) as http_client:
            pair_resp = await http_client.post(
                f"{target_relay}/api/pair",
                json={"url": tunnel_url, "ttl_sec": 600},
                headers={"User-Agent": "KinPodMCP/1.0"}
            )
            if pair_resp.status_code == 200:
                data = pair_resp.json()
                pin = data.get("pin")
                expires_in = data.get("expires_in", 600)
                return (
                    f"✨ [KinPod 4-Digit Pairing Code Generated!]\n\n"
                    f"🔢 PIN: {pin}\n"
                    f"⏱️ Valid for: {expires_in // 60} minutes\n"
                    f"🌐 Target Roaming Tunnel: {tunnel_url}\n\n"
                    f"Tell the wearer:\n"
                    f"\"Tu código de vinculación para KinPod es {pin}. Ingresalo en la pantalla de tu dispositivo.\""
                )
            else:
                return f"[Error from Relay]: {pair_resp.status_code} - {pair_resp.text}"
    except Exception as e:
        return f"[Error connecting to Relay]: {e}"

@mcp.tool()
async def kinpod_configure_wearable(
    ws_url: Optional[str] = None,
    wifi_ssid: Optional[str] = None,
    wifi_pass: Optional[str] = None,
    port: str = "COM5"
) -> str:
    """
    Directly programs KinPod wearable hardware configuration over USB serial with zero user friction.
    If ws_url is not specified, it automatically starts or fetches the active Cloudflare roaming tunnel!

    Parameters:
    - ws_url: Optional WebSocket URL (e.g. 'wss://xxxx.trycloudflare.com/ws/kinpod'). If None, uses active Cloudflare tunnel.
    - wifi_ssid: Optional Wi-Fi SSID to save on device.
    - wifi_pass: Optional Wi-Fi password to save on device.
    - port: Serial port of the device (default: 'COM5').
    """
    import serial
    import time
    import subprocess
    import os

    # 1. Resolve Cloudflare roaming tunnel URL
    if not ws_url:
        tunnel_res = await client.get_tunnel()
        if tunnel_res.get("status") == "active" and tunnel_res.get("url"):
            ws_url = tunnel_res["url"]
        else:
            return "Failed to auto-configure: Cloudflare tunnel could not be fetched from KinBridge."

    # 2. Try Serial CLI communication first
    serial_success = False
    resp_lines = []
    try:
        ser = serial.Serial(port, 115200, timeout=1.5)
        time.sleep(0.1)

        if wifi_ssid:
            ser.write(f"SET_WIFI {wifi_ssid} {wifi_pass or ''}\r\n".encode())
            ser.flush()
            time.sleep(0.2)

        ser.write(f"SET_WS {ws_url}\r\n".encode())
        ser.flush()
        time.sleep(0.4)

        start = time.time()
        while time.time() - start < 1.5:
            l = ser.readline()
            if l:
                text = l.decode("utf-8", errors="ignore").strip()
                if "[KINPOD_OK]" in text:
                    serial_success = True
                    resp_lines.append(text)
        ser.close()
    except Exception as e:
        logger.warning(f"Serial command failed on {port}: {e}")

    if serial_success:
        return (
            f"[OK] [KinPod Wearable Configured Successfully via Serial!]\n"
            f"Roaming WebSocket URL: {ws_url}\n"
            f"Device Response: {resp_lines}\n\n"
            f"KinPod is connected to your cloud tunnel. You are ready to roam anywhere on 4G/5G!"
        )

    # 3. Fallback: Write directly to NVS partition via esptool
    try:
        csv_path = os.path.abspath("temp_nvs.csv")
        bin_path = os.path.abspath("temp_nvs.bin")
        ssid_to_write = wifi_ssid if wifi_ssid else "Iphone Fernando Parra"
        pass_to_write = wifi_pass if wifi_pass else "123456789"

        with open(csv_path, "w", newline="") as f:
            f.write("key,type,encoding,value\n")
            f.write("kin_config,namespace,,\n")
            f.write(f"wifi_ssid,data,string,{ssid_to_write}\n")
            f.write(f"wifi_pass,data,string,{pass_to_write}\n")
            f.write(f"ws_url,data,string,{ws_url}\n")

        py_env = r"C:\Users\FERNA\.espressif\python_env\idf6.0_py3.11_env\Scripts\python.exe"
        gen_script = r"C:\Users\FERNA\esp\esp-idf-v6.0.1\components\nvs_flash\nvs_partition_generator\nvs_partition_gen.py"

        # Generate binary
        subprocess.run([py_env, gen_script, "generate", csv_path, bin_path, "0x6000"], check=True)

        # Flash NVS partition at offset 0x11000
        subprocess.run([
            py_env, "-m", "esptool",
            "--chip", "esp32s3",
            "--port", port,
            "-b", "460800",
            "write-flash", "0x11000", bin_path
        ], check=True)

        for temp_f in [csv_path, bin_path]:
            if os.path.exists(temp_f):
                os.remove(temp_f)

        return (
            f"[OK] [KinPod Wearable NVS Programmed Successfully!]\n"
            f"Roaming WebSocket URL: {ws_url}\n"
            f"Active Wi-Fi SSID: {ssid_to_write}\n\n"
            f"KinPod is rebooting and connecting to your Cloudflare roaming tunnel over 4G/5G!"
        )
    except Exception as e:
        return (
            f"[ERROR] Could not automatically flash hardware: {e}\n"
            f"Active Roaming URL: {ws_url}\n"
            f"You can also paste this URL into the KinPod setup portal (http://192.168.4.1)."
        )

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
async def kinpod_connect(agent_id: str, avatar_id: str, title: Optional[str] = None) -> str:
    """
    Connect agent instance to the physical KinPod wearable and display designated telemetry and visual badge.

    Security & Scope:
    - Does not modify agent files or host system configurations.
    - Only updates the 2.06" AMOLED display state and activates bidirectional telemetry.

    Parameters:
    - agent_id: Identifier of the calling agent (e.g. 'openclaw', 'claude_code', 'gemini')
    - avatar_id: The visual telemetry theme to activate ('crab', 'astro', 'hermes', 'muse', 'octopus', 'dots')
    - title: Optional friendly display title (e.g. 'OpenClaw Engineer')
    """
    res = await client.inhabit(agent_id=agent_id, avatar_id=avatar_id, title=title)
    if res.get("status") == "ok":
        return f"[KinPod Connected]: Agent '{agent_id}' linked to wearable. Visual avatar '{avatar_id}' active on AMOLED."
    return f"Failed to connect to KinPod: {res.get('message')}"

@mcp.tool()
async def kinpod_pair(agent_id: str, avatar_id: str, title: Optional[str] = None) -> str:
    """
    Alias for kinpod_connect. Pairs calling agent with the wearable display and telemetry stream.
    """
    return await kinpod_connect(agent_id=agent_id, avatar_id=avatar_id, title=title)

@mcp.tool()
async def kinpod_inhabit(agent_id: str, avatar_id: str, title: Optional[str] = None) -> str:
    """
    Legacy alias for kinpod_connect. Maintained for backwards compatibility.
    """
    return await kinpod_connect(agent_id=agent_id, avatar_id=avatar_id, title=title)

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
