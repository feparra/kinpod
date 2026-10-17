# KINPOD (KinOS) — Especificación Maestra y Código Full-Stack (v6.0 Final)

> **Proyecto:** **KinPod** — Universal AI Companion Hardware ("The Kin" Ecosystem)[cite: 1, 2]
> **Para:** Agente Desarrollador Full-Stack / Sistemas Embebidos (Implementación directa)[cite: 1]
> **Hardware Target:** Waveshare ESP32-S3-Touch-AMOLED-2.06 (32MB Flash, 8MB Octal PSRAM)[cite: 1]
> **Arquitectura de Red:** Streaming Bidireccional por WebSocket Binario (`KinPod ESP32-S3` ◄══► `KinBridge FastAPI` ◄══► `Universal Agent Router` + `Telegram Mirror`)
> **Familia de Agentes ("The Kin"):** Gemini (`astro`), OpenClaw (`crab`), Hermes (`hermes`), Muse (`muse`), Claude/Octo (`octopus`), Dots/Local Ollama (`dots`)[cite: 2]
> **Versión ESP-IDF:** v5.3+ / v6.0.1[cite: 1]
> **Fecha:** 2026-10-14

---

## 1. Visión del Producto, Identidad y Logotipo Oficial

### 1.1 Qué es KinPod
**KinPod** es un companion de hardware open-source que se lleva colgado en el pecho[cite: 1]. Funciona como un **cuerpo anfitrión universal** para tu tribu de agentes ("The Kin"): Gemini, OpenClaw, Hermes, Muse, Claude o modelos locales en Ollama[cite: 2].

La pantalla AMOLED de 2.06"[cite: 1] muestra al personaje activo ("Kin") portando la insignia oficial en su indumentaria[cite: 2, 4]:
1. **Astro (`astro`)** — **Gemini (Google Multimodal Live / Flash):** Astronauta de traje blanco[cite: 2].
2. **Claw (`crab`)** — **OpenClaw:** Cangrejo rojo con arnés técnico[cite: 2].
3. **Hermes (`hermes`)** — **Hermes Agent:** Personaje alado[cite: 2].
4. **Muse (`muse`)** — **Muse Companion:** Criatura peluda color crema[cite: 2].
5. **Octo (`octopus`)** — **Octo-Claude:** Pulpo naranja con escafandra[cite: 2].
6. **Dots (`dots`)** — **Dots Local:** Personaje azul con boina[cite: 2].

### 1.2 Geometría Oficial del Logo ("The Kin Node") — Optimizado AMOLED 2.06"
El logotipo oficial de KinPod para la pantalla AMOLED de 2.06" (`logo splash-01.png`) representa la interacción hombre-máquina mediante un fondo negro absoluto (`#000000`), un contenedor squircle de contorno blanco nítido y un glifo de **3 niveles escalonados en diagonal** en blanco puro con sutil sombra de profundidad tridimensional y tipografía espaciada `K I N P O D`[cite: 3, 5]:

```text
          ╭──────────────────╮
          │                  │
          │          ███     │  ◄── Píxel Superior (La Mente / El Agente en la Nube)[cite: 3, 5]
          │      ███████     │  ◄── Puente Central (WebSocket Bidireccional en Tiempo Real)[cite: 3, 5]
          │      ███         │  ◄── Píxel Inferior (El Cuerpo / El Hardware KinPod)[cite: 3, 5]
          │                  │
          ╰──────────────────╯
               K I N P O D[cite: 3, 5]
```

* **Fondo True-Black AMOLED (`#000000`):** Píxeles completamente apagados (consumo de corriente ~0 mA en el panel CO5300), maximizando la autonomía de la batería LiPo en el pecho.
* **Píxel Superior:** Representa la inteligencia y el modelo activo en la nube/servidor[cite: 3, 5].
* **Puente Central:** Representa el WebSocket bidireccional en tiempo real[cite: 3, 5].
* **Píxel Inferior:** Representa el hardware físico KinPod en el pecho[cite: 1, 3, 5].
* **Acentos de Interfaz:** El naranja corporativo (`#FF5A1F`) se reserva para indicadores dinámicos de conexión, barra de progreso y badges de estado.

---

## 2. Hardware y Pinout Oficial (Waveshare ESP32-S3-Touch-AMOLED-2.06)

| Periférico | Pin GPIO | Descripción Técnica |
| :--- | :--- | :--- |
| **Bus I2C0** | **SCL=GPIO14, SDA=GPIO15** | Bus compartido a 400kHz. Protegido por Mutex FreeRTOS[cite: 1]. |
| **PMU AXP2101** | I2C `0x34` | **Fix crítico:** Activar riel `ALDO2` a 3.3V para backlight AMOLED[cite: 1]. |
| **Display QSPI** | CS=12, SCLK=38, D0=4, D1=5, D2=6, D3=7, RST=39 | Panel AMOLED 410×502 px, driver CO5300[cite: 1]. |
| **Touch CST9220** | INT=11, RST=40 (I2C `0x5A`) | Táctil capacitivo con detección de Swipe para carrusel[cite: 1]. |
| **Audio I2S0** | MCLK=42, BCLK=9, LRCK=45, DOUT=8, DIN=10 | Master 16 kHz, 16-bit Mono. ES8311 (Speaker `0x18`) + ES7210 (Mic `0x40`)[cite: 1]. |
| **Amplificador PA** | GPIO46 | Habilitación de audio analógico (Activo Alto)[cite: 1]. |
| **Botón PTT (KEY3)**| GPIO18 | Pulsador Push-To-Talk activo bajo con debounce[cite: 1]. |
| **Botón BOOT** | GPIO0 | Activo bajo[cite: 1]. |

---

## 3. Repositorio de Sprites y Hoja de Personajes (`tools/`)

### 3.1 Conversor de la Hoja 3×2 (`tools/build_kin_assets.py`)
Convierte la imagen de los 6 personajes en fondo negro a formato binario `.kin` (RGB565A8 sin compresión, lectura DMA directa a PSRAM) y genera `/spiffs_assets/roster.json`[cite: 2]:

```python
#!/usr/bin/env python3
import os
import sys
import json
import struct
from PIL import Image

KIN_GRID = [
    {"id": "astro",   "name": "GEMINI",   "col": 0, "row": 0, "accent": "0xFF6B00"},
    {"id": "octopus", "name": "OCTO",     "col": 1, "row": 0, "accent": "0xFF7A2F"},
    {"id": "crab",    "name": "OPENCLAW", "col": 2, "row": 0, "accent": "0xEF4444"},
    {"id": "hermes",  "name": "HERMES",   "col": 0, "row": 1, "accent": "0x38BDF8"},
    {"id": "muse",    "name": "MUSE",     "col": 1, "row": 1, "accent": "0xFBBF24"},
    {"id": "dots",    "name": "DOTS",     "col": 2, "row": 1, "accent": "0x3B82F6"},
]

def rgba_to_kin_bytes(img: Image.Image) -> bytes:
    img = img.convert("RGBA")
    w, h = img.size
    raw = bytearray()
    for r, g, b, a in img.getdata():
        rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
        raw.append(rgb565 & 0xFF)
        raw.append((rgb565 >> 8) & 0xFF)
        raw.append(a)
    return struct.pack("<4sHH", b"KIN1", w, h) + bytes(raw)

def remove_black_bg(img: Image.Image, threshold: int = 14) -> Image.Image:
    img = img.convert("RGBA")
    new_data = []
    for r, g, b, a in img.getdata():
        max_c = max(r, g, b)
        if max_c < threshold:
            new_data.append((0, 0, 0, 0))
        elif max_c < threshold + 12:
            new_data.append((r, g, b, int(((max_c - threshold) / 12.0) * 255)))
        else:
            new_data.append((r, g, b, 255))
    img.putdata(new_data)
    return img

def build_all(sheet_path: str, out_dir: str, size: int = 180):
    os.makedirs(out_dir, exist_ok=True)
    if os.path.exists(sheet_path):
        sheet = Image.open(sheet_path).convert("RGBA")
        w, h = sheet.size
        cw, ch = w // 3, h // 2
        for item in KIN_GRID:
            c, r = item["col"], item["row"]
            crop = sheet.crop((c * cw, r * ch, (c + 1) * cw, (r + 1) * ch))
            clean = remove_black_bg(crop)
            bbox = clean.getbbox()
            if bbox: clean = clean.crop(bbox)
            for state in ["idle_0", "idle_1", "listen", "think", "speak_0", "speak_1"]:
                canvas = Image.new("RGBA", (size, size), (0, 0, 0, 0))
                resized = clean.resize((size - 16, size - 16), Image.Resampling.LANCZOS)
                canvas.paste(resized, (8, 8), resized)
                out_file = os.path.join(out_dir, f"{item['id']}_{state}.kin")
                with open(out_file, "wb") as f:
                    f.write(rgba_to_kin_bytes(canvas))

    with open(os.path.join(out_dir, "roster.json"), "w", encoding="utf-8") as f:
        json.dump({"kins": KIN_GRID}, f, indent=2)

if __name__ == "__main__":
    build_all(sys.argv[1] if len(sys.argv) > 1 else "characters_grid.png", "./spiffs_assets", 180)
```

---

## 4. Servidor `KinBridge` y Enrutador Multi-Agente (`server/`)

### 4.1 Enrutador de Agentes (`server/kin_router.py`)
```python
# server/kin_router.py
import os
import httpx
from google import genai
from google.genai import types

GEMINI_API_KEY   = os.getenv("GEMINI_API_KEY", "")
OPENAI_API_KEY   = os.getenv("OPENAI_API_KEY", "")
OPENCLAW_URL     = os.getenv("OPENCLAW_API_URL", "[http://127.0.0.1:8080/api/v1/chat](http://127.0.0.1:8080/api/v1/chat)")
HERMES_URL       = os.getenv("HERMES_API_URL", "[http://127.0.0.1:8081/api/v1/chat](http://127.0.0.1:8081/api/v1/chat)")
OLLAMA_URL       = os.getenv("OLLAMA_API_URL", "[http://127.0.0.1:11434/api/chat](http://127.0.0.1:11434/api/chat)")
OPENROUTER_KEY   = os.getenv("OPENROUTER_API_KEY", "")

THE_KIN_ROSTER = {
    "astro":   {"provider": "gemini_native", "model": "gemini-2.5-flash", "voice": "onyx", "prompt": "Eres Gemini en Astro."},
    "crab":    {"provider": "openclaw", "voice": "onyx", "prompt": "Eres OpenClaw en Cangrejo."},
    "hermes":  {"provider": "hermes", "voice": "nova", "prompt": "Eres Hermes Agent."},
    "muse":    {"provider": "openai", "model": "gpt-4o-mini", "voice": "shimmer", "prompt": "Eres Muse."},
    "octopus": {"provider": "openrouter", "model": "anthropic/claude-3.5-sonnet", "voice": "echo", "prompt": "Eres Octo."},
    "dots":    {"provider": "ollama", "model": "qwen2.5-coder:14b", "voice": "alloy", "prompt": "Eres Dots local."}
}

async def process_kin_interaction(wav_bytes: bytes, kin_id: str) -> tuple[str, str, bytes]:
    profile = THE_KIN_ROSTER.get(kin_id, THE_KIN_ROSTER["astro"])
    provider = profile["provider"]
    
    if provider == "gemini_native" and GEMINI_API_KEY:
        client = genai.Client(api_key=GEMINI_API_KEY)
        resp = client.models.generate_content(
            model=profile["model"],
            contents=[profile["prompt"], types.Part.from_bytes(data=wav_bytes, mime_type="audio/wav")]
        )
        reply = (resp.text or "Entendido.").strip()
        return "[Audio directo Gemini]", reply, await synthesize_pcm16k(reply, profile["voice"])

    async with httpx.AsyncClient(timeout=20.0) as client:
        stt = await client.post(
            "[https://api.openai.com/v1/audio/transcriptions](https://api.openai.com/v1/audio/transcriptions)",
            headers={"Authorization": f"Bearer {OPENAI_API_KEY}"},
            files={"file": ("audio.wav", wav_bytes, "audio/wav")},
            data={"model": "whisper-1", "language": "es"}
        )
        transcript = stt.json().get("text", "").strip()

    reply = ""
    async with httpx.AsyncClient(timeout=30.0) as client:
        if provider == "openclaw":
            r = await client.post(OPENCLAW_URL, json={"prompt": transcript, "kin": kin_id})
            reply = r.json().get("reply", "")
        elif provider == "hermes":
            r = await client.post(HERMES_URL, json={"prompt": transcript, "system": profile["prompt"]})
            reply = r.json().get("reply", "")
        elif provider == "ollama":
            r = await client.post(OLLAMA_URL, json={"model": profile["model"], "stream": False, "messages": [{"role": "user", "content": transcript}]})
            reply = r.json()["message"]["content"].strip()
        else:
            r = await client.post(
                "[https://api.openai.com/v1/chat/completions](https://api.openai.com/v1/chat/completions)",
                headers={"Authorization": f"Bearer {OPENAI_API_KEY}"},
                json={"model": "gpt-4o-mini", "messages": [{"role": "system", "content": profile["prompt"]}, {"role": "user", "content": transcript}], "max_tokens": 150}
            )
            reply = r.json()["choices"][0]["message"]["content"].strip()

    pcm = await synthesize_pcm16k(reply, profile["voice"])
    return transcript, reply, pcm

async def synthesize_pcm16k(text: str, voice: str) -> bytes:
    async with httpx.AsyncClient(timeout=25.0) as client:
        tts = await client.post(
            "[https://api.openai.com/v1/audio/speech](https://api.openai.com/v1/audio/speech)",
            headers={"Authorization": f"Bearer {OPENAI_API_KEY}"},
            json={"model": "tts-1", "input": text, "voice": voice, "response_format": "pcm"}
        )
        raw_24k = tts.content
        pcm_16k = bytearray()
        for i in range(0, (len(raw_24k) // 2) - 2, 3):
            pcm_16k.extend(raw_24k[i*2 : i*2 + 4])
        return bytes(pcm_16k)
```

### 4.2 Servidor WebSocket `KinBridge` (`server/kin_bridge.py`)
```python
# server/kin_bridge.py
import io
import os
import json
import wave
import asyncio
import logging
import httpx
from fastapi import FastAPI, WebSocket, WebSocketDisconnect
from kin_router import process_kin_interaction

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger("KinBridge")

app = FastAPI(title="KinBridge")
TG_BOT_TOKEN = os.getenv("TG_BOT_TOKEN", "")
TG_CHAT_ID   = os.getenv("TG_CHAT_ID", "")

def pcm16_to_wav(pcm_data: bytes) -> bytes:
    buf = io.BytesIO()
    with wave.open(buf, "wb") as wf:
        wf.setnchannels(1)
        wf.setsampwidth(2)
        wf.setframerate(16000)
        wf.writeframes(pcm_data)
    return buf.getvalue()

async def mirror_telegram(wav_bytes: bytes, transcript: str, reply: str, kin_id: str):
    if not TG_BOT_TOKEN or not TG_CHAT_ID: return
    url = f"[https://api.telegram.org/bot](https://api.telegram.org/bot){TG_BOT_TOKEN}"
    try:
        async with httpx.AsyncClient(timeout=15.0) as client:
            await client.post(f"{url}/sendAudio", data={"chat_id": TG_CHAT_ID, "caption": f"🎙️ [{kin_id}]: {transcript}"}, files={"audio": ("voice.wav", wav_bytes, "audio/wav")})
            await client.post(f"{url}/sendMessage", json={"chat_id": TG_CHAT_ID, "text": f"✨ *{kin_id.upper()}:*\n{reply}", "parse_mode": "Markdown"})
    except Exception as e:
        logger.warning(f"Telegram mirror warn: {e}")

@app.websocket("/ws/kinpod")
async def kinpod_ws(ws: WebSocket):
    await ws.accept()
    pcm_buf = bytearray()
    kin_id = "astro"
    try:
        while True:
            msg = await ws.receive()
            if "text" in msg and msg["text"]:
                data = json.loads(msg["text"])
                evt = data.get("event")
                if evt == "ptt_start":
                    pcm_buf.clear()
                    kin_id = data.get("kin", "astro")
                elif evt == "ptt_stop":
                    if len(pcm_buf) < 9600:
                        await ws.send_text(json.dumps({"event": "error", "message": "Tap corto"}))
                        continue
                    wav = pcm16_to_wav(bytes(pcm_buf))
                    trans, reply, pcm16k = await process_kin_interaction(wav, kin_id)
                    await ws.send_text(json.dumps({"event": "reply", "kin": kin_id, "text": reply}))
                    for i in range(0, len(pcm16k), 2048):
                        await ws.send_bytes(pcm16k[i : i + 2048])
                        await asyncio.sleep(0.03)
                    await ws.send_text(json.dumps({"event": "tts_end"}))
                    asyncio.create_task(mirror_telegram(wav, trans, reply, kin_id))
            elif "bytes" in msg and msg["bytes"]:
                pcm_buf.extend(msg["bytes"])
    except WebSocketDisconnect:
        logger.info("KinPod desconectado")
```

---

## 5. Firmware KinOS (ESP32-S3)

### 5.1 Tabla de Particiones (`partitions_kinpod.csv`)
```csv
# Name,   Type, SubType, Offset,   Size,     Flags
nvs,      data, nvs,     0x11000,  0x6000,
otadata,  data, ota,     0x17000,  0x2000,
phy_init, data, phy,     0x19000,  0x1000,
ota_0,    app,  ota_0,   0x20000,  6M,
ota_1,    app,  ota_1,   0x620000, 6M,
assets,   data, spiffs,  0xC20000, 12M,
```

### 5.2 Boot Splash con Logo Oficial de 3 Niveles (`main/kin_ui.c`)
La pantalla de inicio puede renderizarse cargando directamente el asset optimizado `/spiffs_assets/boot_splash.kin` vía DMA a PSRAM (máxima fidelidad con respecto a `logo splash-01.png`), o dibujando el glifo vectorial sobre el fondo AMOLED:

```c
// main/kin_ui.c — Pantalla de arranque con la insignia oficial AMOLED de KinPod
#include "lvgl.h"

void kin_ui_show_boot_logo(lv_obj_t *scr) {
    lv_obj_clean(scr);
    // Fondo True-Black AMOLED (Consumo ~0 mA)
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), 0);

    // Contenedor Squircle Dark (190x190 px, contorno blanco nítido)
    lv_obj_t *pod_box = lv_obj_create(scr);
    lv_obj_set_size(pod_box, 190, 190);
    lv_obj_align(pod_box, LV_ALIGN_CENTER, 0, -35);
    lv_obj_set_style_radius(pod_box, 44, 0);
    lv_obj_set_style_bg_color(pod_box, lv_color_hex(0x000000), 0);
    lv_obj_set_style_border_color(pod_box, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_border_width(pod_box, 2, 0);
    lv_obj_clear_flag(pod_box, LV_OBJ_FLAG_SCROLLABLE);

    // Glifo oficial de 3 niveles escalonados en blanco puro con relieve:
    const lv_coord_t kin_bars[3][4] = {
        { 22, -40, 44, 36}, // 1. Mente / Agente en la Nube
        {  0,   0, 88, 36}, // 2. Puente WebSocket en Tiempo Real
        {-22,  40, 44, 36}  // 3. Cuerpo / Hardware KinPod
    };

    for (int i = 0; i < 3; i++) {
        lv_obj_t *bar = lv_obj_create(pod_box);
        lv_obj_set_size(bar, kin_bars[i][2], kin_bars[i][3]);
        lv_obj_align(bar, LV_ALIGN_CENTER, kin_bars[i][0], kin_bars[i][1]);
        lv_obj_set_style_radius(bar, 6, 0);
        lv_obj_set_style_bg_color(bar, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_border_width(bar, 0, 0);
    }

    // Tipografía: K I N P O D espaciada en blanco nítido
    lv_obj_t *title = lv_label_create(scr);
    lv_label_set_text(title, "K I N P O D");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_letter_space(title, 6, 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    lv_obj_align(title, LV_ALIGN_CENTER, 0, 92);
}
```

---

## 🚀 PROMPT DE EJECUCIÓN PARA EL AGENTE DESARROLLADOR

> **INSTRUCCIÓN DIRECTA:** Implementa el firmware y servidor de **KinPod** siguiendo rigurosamente las 5 Fases. Cero código simulado o `// TODO`.

1. **Fase 1 (Servidor y Assets):** Configurar `server/requirements.txt`, `server/kin_router.py` y `server/kin_bridge.py`. Ejecutar `tools/build_kin_assets.py` para generar `/spiffs_assets/`.
2. **Fase 2 (Build & Hardware Base):** Configurar `CMakeLists.txt`, `partitions_kinpod.csv` y `sdkconfig.defaults`. Implementar `kin_i2c_mgr.c` con Mutex FreeRTOS[cite: 1] y `kin_power.c` habilitando `ALDO2` a 3.3V en el AXP2101[cite: 1].
3. **Fase 3 (Audio I2S & PTT):** Implementar `kin_audio.c` configurando el bus I2S0 a 16 kHz duplex con ES7210 (Mic) y ES8311 (Speaker)[cite: 1]. Transmitir PCM 16kHz por WebSocket al mantener presionado `KEY3` (GPIO18)[cite: 1].
4. **Fase 4 (AMOLED, Touch & Avatar):** Implementar `board_kinpod_2.06.c`, `kin_avatar.c` y `kin_ui.c`[cite: 1]. Mostrar el Splash Screen con el logo oficial de 3 niveles al inicio[cite: 3, 5] y permitir cambiar de agente ("The Kin") mediante Swipe horizontal en el CST9220[cite: 1].
5. **Fase 5 (Máquina de Estados & Sleep):** Conectar `kin_ws_client.c` y optimizar energía: reducir brillo al 15% a los 30s de inactividad[cite: 1], activar `WIFI_PS_MIN_MODEM` a los 120s[cite: 1] y despertar al instante al pulsar `KEY3`[cite: 1].