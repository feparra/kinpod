# 🦊 KinPod (KinOS) — Universal AI Companion Hardware

> **Universal Physical Body for "The Kin" Tribe of AI Agents**  
> *Gemini (`astro`) • OpenClaw (`crab`) • Hermes (`hermes`) • Muse (`muse`) • Claude (`octopus`) • Dots (`dots`)*

KinPod is an open-source wearable AI companion hardware worn on the chest. Powered by an ESP32-S3 and a 2.06" Touch AMOLED display, it bridges human intent with autonomous AI agents via real-time WebSocket binary streaming and the **Model Context Protocol (MCP)**.

---

## ⚡ Highlights

- **AMOLED 2.06" Screen:** 410×502 px high-density panel (CO5300 QSPI driver) with true-black power saving.
- **MCP Native Integration:** Autonomous agents like **OpenClaw** can discover, inhabit, speak, and request Human-In-The-Loop approvals directly through KinPod.
- **Full Duplex Audio:** ES7210 microphone + ES8311 speaker with NS4150 amplifier (I2S0 16kHz 16-bit mono).
- **Physical Push-To-Talk:** KEY3 (GPIO18) tactile button with <300ms short-tap rejection.
- **Power Management:** 30s auto-dimming to 15% brightness, 120s modem sleep, <100ms instant wake-up.
- **Multi-Agent Roster:** Instant swipe carousel to switch between Gemini, OpenClaw, Hermes, Muse, Claude, and Ollama.

---

## 📐 Hardware Specifications (Waveshare ESP32-S3-Touch-AMOLED-2.06)

| Peripheral | GPIO Pins | Description |
| :--- | :--- | :--- |
| **I2C0 Bus** | SCL=14, SDA=15 | Shared 400kHz bus with FreeRTOS Mutex |
| **PMU AXP2101** | I2C `0x34` | Critical: ALDO2 power rail set to 3.3V for AMOLED backlight |
| **QSPI AMOLED** | CS=12, SCLK=38, D0=4, D1=5, D2=6, D3=7, RST=39 | 410×502 px, CO5300 driver |
| **CST9220 Touch**| INT=11, RST=40 (I2C `0x5A`) | Capacitive touch with horizontal swipe detection |
| **I2S0 Audio** | MCLK=42, BCLK=9, LRCK=45, DOUT=8, DIN=10 | Master 16 kHz duplex: ES7210 (Mic) + ES8311 (Speaker) |
| **PA Amplifier** | GPIO46 | Active-High audio output enable |
| **PTT Button** | GPIO18 (KEY3) | Push-To-Talk active-low with debounce |

---

## 🚀 Quickstart

### 1. KinBridge Server (FastAPI)
```bash
cd server
pip install -r requirements.txt
python kin_bridge.py
```

### 2. KinPod MCP Server for OpenClaw & Claude
```bash
pip install -e ./mcp
python -m kinpod_mcp.server
```
In your agent configuration (`mcp_config.json`):
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

### 3. Compile SPIFFS Character & Boot Assets
```bash
python tools/build_kin_assets.py IMG_6246.JPG
```

### 4. Interactive Digital Twin Simulator
Open `kinpod_simulator.html` in any modern web browser to test animations, swipe transitions, PTT audio streaming, and push approvals without hardware.

---

## 📄 License
MIT License. Open-source hardware and software.
