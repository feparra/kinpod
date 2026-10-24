# KinPod Model Context Protocol (MCP) Server

Universal physical embodiment protocol for autonomous AI agents (OpenClaw, Claude Code, Gemini, Cursor, Windsurf, Antigravity).

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)
[![Python 3.10+](https://img.shields.io/badge/python-3.10+-blue.svg)](https://www.python.org/downloads/)
[![MCP Specification](https://img.shields.io/badge/MCP-1.0-orange.svg)](https://modelcontextprotocol.io/)

---

## 🔒 Security & Threat Model

The KinPod MCP server is engineered specifically for autonomous agent environments with strict security boundaries:

- **Zero Host Mutation:** Does **not** modify system files, install background services, or mutate the calling agent's internal configuration files.
- **Zero Arbitrary Execution:** Does **not** evaluate code strings, spawn shell sub-processes, or execute arbitrary terminal commands.
- **Bounded Tool Surface:** Exposes strictly 7 deterministic peripheral control and telemetry functions.
- **Isolated Network Scope:** Network access is strictly restricted to HTTP/WebSocket communication with the KinBridge daemon (`127.0.0.1:8000`) or the user's declared cloud relay tunnel.
- **Hardware-Enforced HITL Boundary:** The `kinpod_request_approval` tool enforces a synchronous execution pause that requires physical tactile human contact on the chest touchscreen (`[APPROVE]` / `[REJECT]`), preventing autonomous unverified actions.

---

## 🛠️ Available MCP Tools

| Tool | Parameters | Description | Security Scope |
| :--- | :--- | :--- | :--- |
| `kinpod_status` | None | Returns battery mV, Wi-Fi RSSI, link state, and active visual theme. | Read-only telemetry |
| `kinpod_connect` | `agent_id`, `avatar_id`, `title` | Connects agent to the wearable, activating display badge & telemetry. | Display & telemetry |
| `kinpod_pair` | `agent_id`, `avatar_id`, `title` | Alias for `kinpod_connect`. | Display & telemetry |
| `kinpod_speak` | `text`, `voice` | Synthesizes and streams audio waveform to ES8311 speaker codec. | Audio output |
| `kinpod_request_approval` | `action`, `details`, `risk`, `timeout_sec` | **Human-In-The-Loop:** Pauses agent execution until human taps Approve/Reject on chest touchscreen. | Security verification |
| `kinpod_expose_tunnel` | `port` | Spawns a secure Cloudflare tunnel for outdoor cellular hotspot roaming. | Tunnel registration |
| `kinpod_generate_pairing_code` | `relay_url` | Generates a temporary 4-digit pairing PIN for zero-cable wearable setup. | Ephemeral OTP |

---

## 🚀 Installation & Registration

### Step 1: Install Package (Operator Command)
```bash
cd kinpod/mcp
pip install -e .
```

### Step 2: Register with your Agent Platform

#### For OpenClaw CLI
```bash
openclaw mcp add kinpod -- python -m kinpod_mcp.server
```

#### For Claude Code
```bash
claude mcp add kinpod python -m kinpod_mcp.server
```

#### For Generic `mcp_config.json`
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

---

## 📄 License
MIT License. Open-source hardware and software.
