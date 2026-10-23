# KinPod Project Status & Engineering Roadmap

> **Target Hardware:** Waveshare ESP32-S3 Touch AMOLED 2.06" (CO5300 + CST9220/FT3168, ES7210 Mic, ES8311 Speaker, NS4150 Amp)  
> **Host Environment:** Windows (COM5 @ 115200 / 460800 flash)  
> **Server Hub:** `server/kin_bridge.py` on `ws://127.0.0.1:8000/ws/kinpod`  
> **Last Updated:** October 2026  

---

## 📌 Phase 1 to 3 Status: Completed Milestones

### 1. Zero-Friction 4-Digit PIN Pairing (Cloud Rendezvous / OTP)
- [x] Cloud Relay microservice (`relay/relay_server.py`) and Cloudflare Anycast tunnel (`https://relay.thekinpod.com/api/pair`).
- [x] AMOLED numeric keypad (obsidian theme, 3x4) rendered in Octal PSRAM at 60 FPS (`kin_pairing.c`).
- [x] Fixed TLS handshake negotiation in ESP-IDF mbedTLS (SNI + server cert validation).
- [x] Roaming Wi-Fi & cloud pairing verified in hardware.

### 2. Full-Duplex Bidirectional Audio Pipeline
- [x] Physical BOOT button (GPIO 0) active-low PTT trigger.
- [x] ES7210 dual I2S microphone streaming 16 kHz 16-bit PCM to `kin_bridge.py`.
- [x] Real-time RMS VU level visualization on AMOLED HUD during `LISTENING`.
- [x] Hand attenuation tolerance on Wi-Fi antenna (`s_pending_ptt_stop` queue mechanism).
- [x] Autonomous speech synthesis playback via ES8311 codec and NS4150 amplifier (`SPEAKING`).

### 3. Human-In-The-Loop (HITL) Physical Touch Approval Subsystem
- [x] Obsidian modal UI rendered in PSRAM: grabber pill, severity badge (`CRITICAL`/`HIGH`/`MEDIUM`/`LOW`), action card with details word-wrap, tactile decision buttons (`[X REJECT]`, `[OK APPROVE]`).
- [x] Urgent chime alert (G5 784 Hz -> C6 1046 Hz) and double haptic pulse (40ms + 60ms gap + 40ms) on popup.
- [x] 60-second autonomous timeout watchdog (`approval_timeout_task`) with auto-rejection.
- [x] **Touch Responsiveness Overhaul:**
  - Instant trigger on touch-down (zero latency, no 400ms release requirement).
  - Fallback trigger on touch-up for drag interactions.
  - Full bottom-half screen hitboxes (`y >= 300`: Left `<= 205` = REJECT, Right `> 205` = APPROVE).
  - Scaled typography for "REQUIRES HUMAN CONFIRMATION" (Scale 2, high contrast).

---

## 🗺️ Upcoming Roadmap & Backlog Items

### 🎨 Item 1: Avatar Facial Animations & Morphing (High Priority)
* **Problem:** Currently, the Kin avatar screen displays a static profile card (`avatar_<kin>.png/raw`). When transitioning between `LISTENING`, `THINKING`, and `SPEAKING`, only the text HUD / status pill changes, while the avatar character remains static.
* **Requirements:**
  1. **Idle Animation:** Subtle breathing or eye blinking every 3–5 seconds using double-buffered sub-region blits in Octal PSRAM.
  2. **Listening State:** Character ears/eyes alert, reactive to live microphone RMS volume bars.
  3. **Thinking State:** Orbiting/pulsing aura or thinking expression while Gemini/Whisper processes.
  4. **Speaking State:** Mouth morphing frames (open/mid/closed phonemes) synchronized to incoming 16 kHz audio energy envelopes.
  5. **Asset Pipeline:** Multi-frame raw spritesheet generator in `tools/build_avatar_screens.py`.

### ⚡ Item 2: Low-Power Sleep & Screen Wake Optimization
- [ ] Auto-dimming AMOLED display to 15% brightness after 30 seconds of inactivity.
- [ ] Modem sleep (`WIFI_PS_MIN_MODEM`) after 120 seconds of silence.
- [ ] Instant hardware wake-up on BOOT button press (<50 ms).

### 🤖 Item 3: Local Offline Mode (Ollama / Small Audio Cues)
- [ ] Offline audio prompts stored in SPIFFS when Wi-Fi is disconnected.
- [ ] Quick local fallback for battery telemetry and Kin switching.
