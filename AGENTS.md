# AGENTS.md — KinPod Protocol for Autonomous AI Agents

> **For:** Autonomous Agents (OpenClaw, Claude Code, Cursor, Windsurf, Antigravity)  
> **Interface:** Model Context Protocol (MCP)  
> **Physical Target:** KinPod Wearable (Waveshare ESP32-S3 Touch AMOLED 2.06")

---

## 1. What is KinPod?

**KinPod** is an open-source wearable AI companion body worn on the chest. It gives your AI agent physical embodiment:
- **Face:** AMOLED 2.06" (410×502 px) showing your animated agent character ("Kin").
- **Ears:** ES7210 microphone capturing user voice via Push-To-Talk (KEY3 GPIO18).
- **Mouth:** ES8311 speaker with NS4150 amplifier speaking aloud.
- **Hands:** CST9220 capacitive touchscreen with Human-In-The-Loop approval buttons.

---

## 2. MCP Quickstart for Agents (e.g. OpenClaw)

If you are an agent given this repository, execute:
```bash
git clone https://github.com/feparra/kinpod.git
cd kinpod/mcp && pip install -e .
```

Then ensure the `kinpod` MCP server is configured in your `mcp_config.json`:
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

Once running:
1. Verify hardware connectivity by calling `kinpod_status()`.
2. Take possession of your physical avatar by calling `kinpod_inhabit(agent_id="openclaw", avatar_id="crab")`.
3. The user's chest display will immediately transition to your Crab character with red accent (`#EF4444`).

---

## 3. MCP Tools Reference

| MCP Tool | Description |
| :--- | :--- |
| `kinpod_status()` | Returns battery voltage, Wi-Fi RSSI, connection state, and active avatar. |
| `kinpod_generate_pairing_code(relay_url)` | **4-Digit Pairing (Zero App, Zero Cable):** Spawns roaming tunnel, registers with Cloud Relay, and gives user a 4-digit PIN (e.g. `4821`) to tap on their KinPod chest screen. |
| `kinpod_expose_tunnel(port)` | **Zero-Config Roaming:** Spawns a public Cloudflare tunnel (`wss://...`) so KinPod can roam outdoors on cellular hotspot. |
| `kinpod_configure_wearable(ws_url, wifi_ssid, wifi_pass, port)` | **Zero-Friction Hardware Config:** Directly configures wearable over USB serial with the Cloudflare tunnel and Wi-Fi credentials. |
| `kinpod_inhabit(agent_id, avatar_id, title)` | Changes the character on the chest screen. |
| `kinpod_speak(text, voice)` | Streams synthesized speech through the hardware chest speaker. |
| `kinpod_request_approval(action, details, risk)` | **Human-In-The-Loop:** Pops up tactile Approve/Reject modal on screen. Pauses execution until wearer taps screen. |

---

## 4. The Kin Character Roster

When inhabiting the device, select your matching avatar:
- **`crab` (OpenClaw):** Red technical crab with cybernetic harness (`#EF4444`).
- **`astro` (Gemini):** White astronaut suit with gold visor (`#FF6B00`).
- **`octopus` (Claude / Quantitative):** Orange octopus with diving helmet (`#FF7A2F`).
- **`hermes` (Hermes Agent):** Winged messenger character (`#38BDF8`).
- **`muse` (Creative / GPT-4o):** Fluffy cream companion creature (`#FBBF24`).
- **`dots` (Ollama Local):** Blue character with beret (`#3B82F6`).

---

## 5. Human-In-The-Loop Best Practice

Whenever about to execute high-risk operations (file deletion, production deployment, financial transactions, shell commands):
```python
# Call kinpod_request_approval before taking action
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
