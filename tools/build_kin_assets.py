#!/usr/bin/env python3
"""
tools/build_kin_assets.py
KinPod Asset Compiler (v6.0 Final)

Converts character grid images and the official boot splash asset into
RGB565A8 raw binary (.kin) files for direct DMA streaming from SPIFFS to AMOLED PSRAM.
"""

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
    """Converts PIL RGBA Image to KinPod .kin binary (Header KIN1 + RGB565 + Alpha byte)."""
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
    """Soft keying for character sprites on black background."""
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

def build_all(sheet_path: str = "IMG_6246.JPG", splash_path: str = "splash_410x502.png", out_dir: str = "./spiffs_assets", size: int = 180):
    os.makedirs(out_dir, exist_ok=True)
    print(f"[*] Building KinPod assets into {out_dir}...")

    # 1. Compile Boot Splash Screen Asset (410x502 AMOLED)
    if os.path.exists(splash_path):
        print(f"[*] Compiling boot splash asset: {splash_path}")
        splash_img = Image.open(splash_path).convert("RGBA")
        splash_kin = rgba_to_kin_bytes(splash_img)
        out_splash = os.path.join(out_dir, "boot_splash.kin")
        with open(out_splash, "wb") as f:
            f.write(splash_kin)
        print(f"    -> Saved {out_splash} ({len(splash_kin)} bytes)")

    # 2. Compile Character Grid
    if os.path.exists(sheet_path):
        print(f"[*] Processing character sheet: {sheet_path}")
        sheet = Image.open(sheet_path).convert("RGBA")
        w, h = sheet.size
        cw, ch = w // 3, h // 2
        for item in KIN_GRID:
            c, r = item["col"], item["row"]
            crop = sheet.crop((c * cw, r * ch, (c + 1) * cw, (r + 1) * ch))
            clean = remove_black_bg(crop)
            bbox = clean.getbbox()
            if bbox:
                clean = clean.crop(bbox)

            for state in ["idle_0", "idle_1", "listen", "think", "speak_0", "speak_1"]:
                canvas = Image.new("RGBA", (size, size), (0, 0, 0, 0))
                resized = clean.resize((size - 16, size - 16), Image.Resampling.LANCZOS)
                canvas.paste(resized, (8, 8), resized)
                out_file = os.path.join(out_dir, f"{item['id']}_{state}.kin")
                with open(out_file, "wb") as f:
                    f.write(rgba_to_kin_bytes(canvas))

    # 3. Write roster.json metadata
    roster_path = os.path.join(out_dir, "roster.json")
    with open(roster_path, "w", encoding="utf-8") as f:
        json.dump({"kins": KIN_GRID, "display": {"width": 410, "height": 502, "driver": "CO5300"}}, f, indent=2)
    print(f"[*] Wrote {roster_path}")
    print("[OK] All assets generated successfully.")

if __name__ == "__main__":
    sheet = sys.argv[1] if len(sys.argv) > 1 else "IMG_6246.JPG"
    build_all(sheet, "splash_410x502.png", "./spiffs_assets", 180)
