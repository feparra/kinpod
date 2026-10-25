#!/usr/bin/env python3
"""
scripts/test_voice_interaction.py
End-to-End Voice Inbound & Transcription Validation for OpenClaw / KinPod
"""

import sys
import os
import asyncio
import time

if sys.stdout.encoding.lower() != 'utf-8':
    sys.stdout.reconfigure(encoding='utf-8')

sys.path.insert(0, os.path.abspath('mcp'))
from kinpod_mcp.client import KinPodClient

async def main():
    client = KinPodClient()

    print("=" * 70)
    print("🎙️ [OpenClaw / KinPod Voice Inbound Validation Test]")
    print("=" * 70)

    # 1. Verify wearable connection
    status = await client.get_status()
    if not status.get("connected"):
        print(f"❌ [ERROR]: KinPod no está conectado o el servidor no responde: {status.get('error')}")
        return

    print(f"✅ KinPod Wearable Online!")
    print(f"   • Avatar activo : {status.get('active_kin', 'crab')}")
    print(f"   • Batería       : {status.get('battery_mv', 3950)} mV")
    print(f"   • Señal Wi-Fi   : {status.get('wifi_rssi', -50)} dBm")
    print("-" * 70)
    print("INSTRUCCIONES PARA LA PRUEBA:")
    print("1. Presioná y MANTENÉ presionado el botón lateral BOOT (GPIO 0) en el KinPod.")
    print("2. Vas a sentir una vibración y ver el indicador rojo LISTENING con vúmetro.")
    print("3. Hablá claro frente al micrófono (ejemplo: 'Hola OpenClaw, confirmando audio').")
    print("4. SOLTÁ el botón para enviar.")
    print("-" * 70)
    print("⏳ OpenClaw está esperando tu audio (Timeout: 35s)...")

    start_time = time.time()
    res = await client.listen(timeout_sec=35, allow_recent_sec=5)
    elapsed = time.time() - start_time

    print("=" * 70)
    if res.get("status") == "heard":
        transcript = res.get("transcript", "").strip()
        print("🎉 [¡ÉXITO TOTAL! OpenClaw recibió tu audio]")
        print(f"🗣️  Texto reconocido: \"{transcript}\"")
        print(f"⏱️  Tiempo de captura/procesamiento: {elapsed:.2f} segundos")
        print("=" * 70)
        print("OpenClaw ya puede recibir y entender todo lo que le hables desde tu KinPod.")
    elif res.get("status") == "timeout":
        print(f"⏱️ [TIMEOUT]: No se detectó pulsación del botón PTT en {elapsed:.1f} segundos.")
        print("Asegurate de mantener presionado el botón lateral BOOT mientras hablás.")
    else:
        print(f"⚠️ [RESULTADO]: {res}")
    print("=" * 70)

if __name__ == "__main__":
    asyncio.run(main())
