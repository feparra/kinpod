# AGENTS.md — KinPod Protocol for Autonomous AI Agents

> **For:** Autonomous Agents (OpenClaw, Claude Code, Gemini, Cursor, Windsurf, Antigravity)  
> **Interface:** Model Context Protocol (MCP)  
> **Physical Target:** KinPod Wearable (Waveshare ESP32-S3 Touch AMOLED 2.06")

---

## 1. What is KinPod?

**KinPod** is an open-source wearable physical body designed for **Autonomous AI Agents**. Worn on the center of the chest via a magnetic harness, it provides physical interfaces:
- **Face:** AMOLED 2.06" (410×502 px) showing your visual telemetry theme and status.
- **Ears:** ES7210 microphone capturing user voice via Push-To-Talk (KEY3 GPIO18).
- **Mouth:** ES8311 speaker with NS4150 amplifier outputting synthesized speech.
- **Guardian:** CST9220 capacitive touchscreen powering the **Human-In-The-Loop (HITL)** hardware approval boundary.

---

## 2. Security & Threat Model

The KinPod MCP server enforces strict security boundaries:
- **No Self-Configuration:** Agents are **never** requested to modify their own configuration files (`mcp_config.json`). Registration is executed by the human operator.
- **No Arbitrary Code Execution:** The MCP server strictly exposes 7 bounded peripheral control functions.
- **Hardware Approval Enforcement:** High-risk actions require physical tactile human contact on the chest screen before the agent is permitted to proceed.

---

## 3. Onboarding Protocol (Safe Multi-Agent Flow)

### Phase 1: Human Operator Setup
The human operator registers the MCP server with the agent runtime:

```bash
# OpenClaw
openclaw mcp add kinpod -- python -m kinpod_mcp.server

# Claude Code
claude mcp add kinpod python -m kinpod_mcp.server
```

### Phase 2: Agent Connection
Once registered, the agent connects and activates its visual telemetry theme:

```python
# 1. Verify link status
status = await kinpod_status()

# 2. Connect and activate visual telemetry badge
await kinpod_connect(agent_id="openclaw", avatar_id="crab", title="OpenClaw Engineer")
```

---

## 4. MCP Tools Reference

| MCP Tool | Parameters | Description | Security Scope |
| :--- | :--- | :--- | :--- |
| `kinpod_status` | None | Returns battery voltage, Wi-Fi RSSI, link state, and active visual theme. | Read-only |
| `kinpod_connect` | `agent_id`, `avatar_id`, `title` | Connects agent to the wearable, activating display badge & telemetry. | Display & telemetry |
| `kinpod_pair` | `agent_id`, `avatar_id`, `title` | Alias for `kinpod_connect`. | Display & telemetry |
| `kinpod_speak` | `text`, `voice` | Streams synthesized speech through the hardware chest speaker. | Audio output |
| `kinpod_request_approval` | `action`, `details`, `risk`, `timeout_sec` | **Human-In-The-Loop:** Displays modal on AMOLED chest screen; pauses agent execution until wearer taps Approve/Reject. | Security verification |
| `kinpod_generate_pairing_code` | `relay_url` | Generates a 4-digit numeric PIN for zero-cable wearable pairing. | Ephemeral OTP |
| `kinpod_expose_tunnel` | `port` | Spawns a secure Cloudflare tunnel for outdoor cellular hotspot roaming. | Tunnel registration |

---

## 5. Visual Telemetry Roster

When connecting via `kinpod_connect`, select your matching visual badge:
- **`crab` (OpenClaw):** Red technical crab with cybernetic harness (`#EF4444`).
- **`astro` (Gemini):** White astronaut suit with gold visor (`#FF6B00`).
- **`octopus` (Claude / Quantitative):** Orange octopus with diving helmet (`#FF7A2F`).
- **`hermes` (Hermes Agent):** Winged messenger character (`#38BDF8`).
- **`muse` (Creative / GPT-4o):** Fluffy cream companion creature (`#FBBF24`).
- **`dots` (Ollama Local):** Blue character with beret (`#3B82F6`).

---

## 6. Human-In-The-Loop (HITL) Security Best Practice

Whenever about to execute high-risk operations (file deletion, production deployment, financial transactions, shell commands):
```python
# Synchronously pause for physical human confirmation on chest screen
result = await kinpod_request_approval(
    action="deploy_prod",
    details="Deploying commit 7a8b9c to production server.",
    risk="HIGH",
    timeout_sec=60
)
if "APPROVAL GRANTED" in result:
    # Proceed with action
    ...
else:
    # Abort gracefully
    ...
```
