# KinPod Model Context Protocol (MCP) Server

Universal physical embodiment protocol for autonomous AI agents (OpenClaw, Claude Code, Cursor, Windsurf, Antigravity).

## Installation

```bash
cd mcp
pip install -e .
```

## Running the Server

Over standard I/O (stdio):
```bash
python -m kinpod_mcp.server
```
or via the installed CLI entrypoint:
```bash
kinpod-mcp
```

## Agent Configuration (`mcp_config.json`)

```json
{
  "mcpServers": {
    "kinpod": {
      "command": "python",
      "args": ["-m", "kinpod_mcp.server"]
    }
  }
}
```

## Available MCP Tools

| Tool | Parameters | Description |
| :--- | :--- | :--- |
| `kinpod_status` | None | Returns battery voltage, RSSI, connection state, and active avatar. |
| `kinpod_inhabit` | `agent_id`, `avatar_id`, `custom_title` | Transitions wearable character to your designated Kin avatar. |
| `kinpod_speak` | `text`, `voice` | Synthesizes and streams voice through wearable speaker. |
| `kinpod_request_approval` | `action`, `details`, `risk`, `timeout_sec` | **Human-In-The-Loop:** Displays modal on AMOLED chest screen; pauses agent execution until wearer taps Approve/Reject. |
| `kinpod_expose_tunnel` | `port` | Automatically creates a zero-config Cloudflare public tunnel for outdoor roaming. |
| `kinpod_generate_pairing_code` | `relay_url` | Generates a 4-digit numeric PIN to link the wearable via the Cloud Relay. |
| `kinpod_configure_wearable` | `ws_url`, `wifi_ssid`, `wifi_pass`, `port` | Directly configures wearable over USB serial with tunnel URL and Wi-Fi credentials. |
