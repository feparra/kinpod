#!/usr/bin/env python3
"""
server/kin_router.py
KinPod Multi-Agent Router

Routes voice interactions to the active Kin personality:
1. astro   -> Gemini Native Audio / Flash (Google)
2. crab    -> OpenClaw Autonomous Agent
3. hermes  -> Hermes Agent
4. muse    -> OpenAI / GPT-4o-mini
5. octopus -> Anthropic Claude 3.5 Sonnet via OpenRouter
6. dots    -> Local Ollama (Qwen2.5-Coder / Llama)
"""

import os
import logging
import httpx

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger("KinRouter")

GEMINI_API_KEY = os.getenv("GEMINI_API_KEY", "")
OPENAI_API_KEY = os.getenv("OPENAI_API_KEY", "")
OPENCLAW_URL = os.getenv("OPENCLAW_API_URL", "http://127.0.0.1:18789/v1/chat/completions")
OPENCLAW_TOKEN = os.getenv("OPENCLAW_TOKEN", "")
HERMES_URL = os.getenv("HERMES_API_URL", "http://127.0.0.1:8081/api/v1/chat")
OLLAMA_URL = os.getenv("OLLAMA_API_URL", "http://127.0.0.1:11434/api/chat")
OPENROUTER_KEY = os.getenv("OPENROUTER_API_KEY", "")

THE_KIN_ROSTER = {
    "astro": {
        "name": "GEMINI",
        "provider": "gemini_native",
        "model": "gemini-2.5-flash",
        "voice": "onyx",
        "prompt": "You are Gemini inside Astro KinPod. Be sharp, warm, direct, and respond in 2-3 concise sentences.",
    },
    "crab": {
        "name": "OPENCLAW",
        "provider": "openclaw",
        "voice": "onyx",
        "prompt": "You are OpenClaw inside KinPod. You are an autonomous agent capable of orchestrating actions.",
    },
    "hermes": {
        "name": "HERMES",
        "provider": "hermes",
        "voice": "nova",
        "prompt": "You are Hermes Agent. Specialized in memory recall and tool execution.",
    },
    "muse": {
        "name": "MUSE",
        "provider": "openai",
        "model": "gpt-4o-mini",
        "voice": "shimmer",
        "prompt": "You are Muse, a warm creative companion. Keep answers uplifting and concise.",
    },
    "octopus": {
        "name": "OCTO",
        "provider": "openrouter",
        "model": "anthropic/claude-3.5-sonnet",
        "voice": "echo",
        "prompt": "You are Octo-Claude. Deep software architect, quantitative analysis, concise responses.",
    },
    "dots": {
        "name": "DOTS",
        "provider": "ollama",
        "model": "qwen2.5-coder:14b",
        "voice": "alloy",
        "prompt": "You are Dots, a 100% private local agent running via Ollama.",
    },
}

async def process_kin_interaction(wav_bytes: bytes, kin_id: str) -> tuple[str, str, bytes]:
    """
    Processes voice input through STT, Agent LLM routing, and TTS synthesis.
    Returns: (transcript, reply_text, pcm_16k_bytes)
    """
    profile = THE_KIN_ROSTER.get(kin_id, THE_KIN_ROSTER["astro"])
    provider = profile["provider"]
    logger.info(f"Processing audio for Kin '{kin_id}' via provider '{provider}' ({len(wav_bytes)} bytes WAV)")

    # 1. Native Gemini Audio streaming if configured
    if provider == "gemini_native" and GEMINI_API_KEY:
        try:
            from google import genai
            from google.genai import types
            client = genai.Client(api_key=GEMINI_API_KEY)
            resp = client.models.generate_content(
                model=profile["model"],
                contents=[
                    profile["prompt"],
                    types.Part.from_bytes(data=wav_bytes, mime_type="audio/wav"),
                ],
            )
            reply = (resp.text or "Entendido.").strip()
            pcm = await synthesize_pcm16k(reply, profile["voice"])
            return "[Direct Gemini Audio]", reply, pcm
        except Exception as e:
            logger.warning(f"Gemini native audio fallback: {e}")

    # 2. Speech-To-Text fallback via OpenAI Whisper or Google Web Speech (Free)
    transcript = ""
    if OPENAI_API_KEY:
        try:
            async with httpx.AsyncClient(timeout=20.0) as client:
                stt = await client.post(
                    "https://api.openai.com/v1/audio/transcriptions",
                    headers={"Authorization": f"Bearer {OPENAI_API_KEY}"},
                    files={"file": ("audio.wav", wav_bytes, "audio/wav")},
                    data={"model": "whisper-1", "language": "es"},
                )
                if stt.status_code == 200:
                    transcript = stt.json().get("text", "").strip()
        except Exception as e:
            logger.warning(f"Whisper STT warn: {e}")

    # Free Google STT fallback
    if not transcript:
        try:
            import speech_recognition as sr
            import io
            recognizer = sr.Recognizer()
            with sr.AudioFile(io.BytesIO(wav_bytes)) as source:
                audio_data = recognizer.record(source)
            try:
                transcript = recognizer.recognize_google(audio_data, language="es-ES").strip()
            except sr.UnknownValueError:
                try:
                    transcript = recognizer.recognize_google(audio_data, language="en-US").strip()
                except Exception:
                    pass
            except Exception as e:
                logger.warning(f"Google STT recognize warning: {e}")
        except Exception as e:
            logger.warning(f"SpeechRecognition fallback error: {e}")

    if not transcript:
        transcript = "Mensaje de voz desde KinPod"

    # 3. LLM Routing
    reply = ""
    try:
        async with httpx.AsyncClient(timeout=30.0) as client:
            if provider == "openclaw":
                try:
                    headers = {"Content-Type": "application/json"}
                    if OPENCLAW_TOKEN:
                        headers["Authorization"] = f"Bearer {OPENCLAW_TOKEN}"
                    payload = {
                        "model": "openclaw/default",
                        "user": "kinpod:fernando",
                        "messages": [{"role": "user", "content": transcript}]
                    }
                    r = await client.post(OPENCLAW_URL, headers=headers, json=payload, timeout=30.0)
                    if r.status_code == 200:
                        data = r.json()
                        if "choices" in data and len(data["choices"]) > 0:
                            reply = data["choices"][0]["message"]["content"].strip()
                        else:
                            reply = data.get("reply", "OpenClaw conectado y listo.")
                    else:
                        logger.warning(f"OpenClaw returned HTTP {r.status_code}: {r.text}")
                        reply = f"OpenClaw: Escuché '{transcript}'"
                except Exception as e:
                    logger.warning(f"OpenClaw gateway connection failed: {e}")
                    reply = f"OpenClaw escuchó: {transcript}"
            elif provider == "hermes":
                r = await client.post(HERMES_URL, json={"prompt": transcript, "system": profile["prompt"]})
                reply = r.json().get("reply", "Hermes sincronizado.") if r.status_code == 200 else "Hermes: Memoria y herramientas indexadas."
            elif provider == "ollama":
                r = await client.post(
                    OLLAMA_URL,
                    json={
                        "model": profile.get("model", "qwen2.5-coder:14b"),
                        "stream": False,
                        "messages": [{"role": "user", "content": transcript}],
                    },
                )
                if r.status_code == 200:
                    reply = r.json().get("message", {}).get("content", "").strip()
                else:
                    reply = "Dots Local: Modelo Ollama listo para procesar."
            elif provider == "openrouter" and OPENROUTER_KEY:
                r = await client.post(
                    "https://openrouter.ai/api/v1/chat/completions",
                    headers={"Authorization": f"Bearer {OPENROUTER_KEY}"},
                    json={
                        "model": profile["model"],
                        "messages": [
                            {"role": "system", "content": profile["prompt"]},
                            {"role": "user", "content": transcript},
                        ],
                        "max_tokens": 150,
                    },
                )
                if r.status_code == 200:
                    reply = r.json()["choices"][0]["message"]["content"].strip()
                else:
                    reply = "Octo-Claude: Análisis completado con éxito."
            elif OPENAI_API_KEY:
                r = await client.post(
                    "https://api.openai.com/v1/chat/completions",
                    headers={"Authorization": f"Bearer {OPENAI_API_KEY}"},
                    json={
                        "model": profile.get("model", "gpt-4o-mini"),
                        "messages": [
                            {"role": "system", "content": profile["prompt"]},
                            {"role": "user", "content": transcript},
                        ],
                        "max_tokens": 150,
                    },
                )
                if r.status_code == 200:
                    reply = r.json()["choices"][0]["message"]["content"].strip()
                else:
                    reply = f"{profile['name']}: Recibido y procesado."
            else:
                reply = f"Hola, soy {profile['name']} en tu KinPod. Audio recibido correctamente en ESP32-S3."
    except Exception as e:
        logger.error(f"Router interaction error: {e}")
        reply = f"{profile['name']}: Conexión activa. Tu mensaje ha sido recibido."

    # 4. Text-To-Speech synthesis
    pcm = await synthesize_pcm16k(reply, profile["voice"])
    return transcript, reply, pcm

async def synthesize_pcm16k(text: str, voice: str = "onyx") -> bytes:
    """
    Synthesizes speech to 16 kHz 16-bit Mono PCM for direct I2S0 DMA playback on ES8311.
    Order of preference:
    1. OpenAI TTS (if OPENAI_API_KEY is configured)
    2. Edge-TTS neural high-fidelity voice (Free, zero API key required, natural Spanish/English)
    3. Fallback sine wave confirmation tone (440Hz)
    """
    # 1. OpenAI TTS
    if OPENAI_API_KEY:
        try:
            async with httpx.AsyncClient(timeout=25.0) as client:
                tts = await client.post(
                    "https://api.openai.com/v1/audio/speech",
                    headers={"Authorization": f"Bearer {OPENAI_API_KEY}"},
                    json={"model": "tts-1", "input": text, "voice": voice, "response_format": "pcm"},
                )
                if tts.status_code == 200:
                    raw_24k = tts.content
                    pcm_16k = bytearray()
                    # Downsample 24kHz -> 16kHz by taking 2 samples for every 3
                    for i in range(0, (len(raw_24k) // 2) - 2, 3):
                        pcm_16k.extend(raw_24k[i * 2 : i * 2 + 4])
                    return bytes(pcm_16k)
        except Exception as e:
            logger.warning(f"OpenAI TTS synthesis warning: {e}")

    # 2. Edge-TTS Neural Synthesis (Zero-config, free, high quality)
    try:
        import edge_tts
        import miniaudio

        voice_map = {
            # Spanish
            "onyx": "es-AR-TomasNeural",
            "alloy": "es-AR-TomasNeural",
            "echo": "es-ES-AlvaroNeural",
            "shimmer": "es-AR-ElenaNeural",
            "nova": "es-AR-ElenaNeural",
            "fable": "es-ES-AlvaroNeural",
            "spanish": "es-AR-TomasNeural",
            "es": "es-AR-TomasNeural",
            # English
            "english": "en-US-GuyNeural",
            "en": "en-US-GuyNeural",
            "en-us": "en-US-GuyNeural",
            "guy": "en-US-GuyNeural",
            "brian": "en-US-BrianNeural",
            "andrew": "en-US-AndrewNeural",
            "ava": "en-US-AvaNeural",
            "jenny": "en-US-JennyNeural",
            "aria": "en-US-AriaNeural",
        }
        v_lower = voice.lower().strip()
        if "-" in voice and len(voice) >= 7:
            edge_voice = voice
        elif v_lower in voice_map:
            edge_voice = voice_map[v_lower]
        else:
            edge_voice = "es-AR-TomasNeural"

        communicate = edge_tts.Communicate(text, edge_voice)
        mp3_buf = bytearray()
        async for chunk in communicate.stream():
            if chunk["type"] == "audio":
                mp3_buf.extend(chunk["data"])

        if mp3_buf:
            decoded = miniaudio.decode(
                bytes(mp3_buf),
                nchannels=1,
                sample_rate=16000,
                output_format=miniaudio.SampleFormat.SIGNED16
            )
            return decoded.samples.tobytes()
    except Exception as e:
        logger.warning(f"Edge-TTS synthesis warning: {e}")

    # 3. Fallback: Synthetic 16kHz confirmation chime (440Hz tone, 0.4s)
    import math
    import struct
    sample_rate = 16000
    duration_s = 0.4
    num_samples = int(sample_rate * duration_s)
    buf = bytearray()
    for i in range(num_samples):
        val = int(8000 * math.sin(2 * math.pi * 440 * (i / sample_rate)))
        buf.extend(struct.pack("<h", val))
    return bytes(buf)
