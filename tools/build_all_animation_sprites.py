#!/usr/bin/env python3
import os
import numpy as np
from PIL import Image
from scipy.ndimage import label, binary_fill_holes, distance_transform_edt

SPRITE_W = 240
SPRITE_H = 240
ART_DIR = r'C:\Users\FERNA\.gemini\antigravity-ide\brain\b98c037a-71c2-4c3e-88c5-6eb9bd470865'

def to_rgb565(im):
    raw = bytearray()
    im_rgb = im.convert('RGB')
    for y in range(im_rgb.height):
        for x in range(im_rgb.width):
            r, g, b = im_rgb.getpixel((x, y))
            rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
            raw.append((rgb565 >> 8) & 0xFF)
            raw.append(rgb565 & 0xFF)
    return raw

def clean_black_bg(img_path, threshold=30):
    im = Image.open(img_path).convert('RGB')
    arr = np.array(im)
    is_dark = (arr[:, :, 0] < threshold) & (arr[:, :, 1] < threshold) & (arr[:, :, 2] < threshold)
    arr[is_dark] = 0
    return Image.fromarray(arr, mode='RGB')

def isolate_white_bg(img_path, seed_points=None, shadow_thresh=0.75):
    im = Image.open(img_path).convert('RGB')
    arr = np.array(im, dtype=np.float32)
    h, w, _ = arr.shape
    r, g, b = arr[:, :, 0], arr[:, :, 1], arr[:, :, 2]
    saturation = np.maximum(np.maximum(r, g), b) - np.minimum(np.minimum(r, g), b)
    y_coords = np.arange(h)[:, None]
    is_floor = (y_coords > shadow_thresh * h) & (saturation < 35)
    is_bg = ((r > 225) & (g > 225) & (b > 225)) | is_floor
    labeled, _ = label(is_bg)
    seeds = [(0, 0), (0, -1), (-1, 0), (-1, -1)]
    if seed_points:
        seeds.extend(seed_points)
    seed_labels = set([labeled[sy, sx] for sy, sx in seeds])
    bg_mask = np.isin(labeled, list(seed_labels))
    fg_mask = binary_fill_holes(~bg_mask)
    dist = distance_transform_edt(fg_mask)
    alpha = np.clip(dist * 255.0 / 2.0, 0, 255).astype(np.uint8)
    alpha_norm = (alpha.astype(np.float32) / 255.0)[:, :, None]
    out_rgb = (arr * alpha_norm).astype(np.uint8)
    return Image.fromarray(out_rgb, mode='RGB')

def process_rgba(img_path):
    im_src = Image.open(img_path).convert('RGBA')
    base = Image.new('RGB', (im_src.width, im_src.height), (0, 0, 0))
    base.paste(im_src, (0, 0), im_src)
    return base

def main():
    spiffs_out = os.path.join('spiffs_assets', 'anim')
    os.makedirs(spiffs_out, exist_ok=True)
    os.makedirs('scratch/sprites', exist_ok=True)

    sources = {
        # --- CRAB (OpenClaw) ---
        'crab_idle': ('rgba', 'characters/crab.png', None),
        'crab_blink': ('white', f'{ART_DIR}/crab_blink_frame_1792613449796.jpg', None),
        'crab_listen': ('white', f'{ART_DIR}/crab_listening_frame_1792613325825.jpg', [(int(0.78 * 1024), 400)]),
        'crab_think': ('black', f'{ART_DIR}/crab_thinking_frame_1792613434497.jpg', None),
        'crab_speak': ('white', f'{ART_DIR}/crab_speaking_frame_1792613308919.jpg', None),

        # --- ASTRO (Gemini) ---
        'astro_idle': ('rgba', 'characters/astro.png', None),
        'astro_blink': ('black', f'{ART_DIR}/astro_blink_frame_1792625308188.jpg', None),
        'astro_listen': ('black', f'{ART_DIR}/astro_listen_frame_1792625325551.jpg', None),
        'astro_think': ('black', f'{ART_DIR}/astro_think_frame_1792625345195.jpg', None),
        'astro_speak': ('white', f'{ART_DIR}/astro_speak_frame_1792625361697.jpg', None),

        # --- HERMES (Hermes Agent) ---
        'hermes_idle': ('rgba', 'characters/hermes.png', None),
        'hermes_blink': ('black', f'{ART_DIR}/hermes_blink_frame_1792625381643.jpg', None),
        'hermes_listen': ('white', f'{ART_DIR}/hermes_listen_clean_1792625462541.jpg', None),
        'hermes_think': ('black', f'{ART_DIR}/hermes_think_frame_1792625422866.jpg', None),
        'hermes_speak': ('white', f'{ART_DIR}/hermes_speak_frame_1792625441877.jpg', None),

        # --- MUSE (Creative) ---
        'muse_idle': ('rgba', 'characters/muse.png', None),
        'muse_blink': ('scratch', 'scratch/sprites/muse_clean_blink.png', None),
        'muse_listen': ('rgba', 'characters/muse.png', None),
        'muse_think': ('rgba', 'characters/muse.png', None),
        'muse_speak': ('rgba', 'characters/muse.png', None),

        # --- OCTOPUS (Octo-Claude) ---
        'octopus_idle': ('rgba', 'characters/octopus.png', None),
        'octopus_blink': ('rgba', 'characters/octopus.png', None),
        'octopus_listen': ('rgba', 'characters/octopus.png', None),
        'octopus_think': ('rgba', 'characters/octopus.png', None),
        'octopus_speak': ('rgba', 'characters/octopus.png', None),

        # --- DOTS (Ollama Local) ---
        'dots_idle': ('rgba', 'characters/dots.png', None),
        'dots_blink': ('scratch', 'scratch/sprites/dots_clean_blink.png', None),
        'dots_listen': ('rgba', 'characters/dots.png', None),
        'dots_think': ('rgba', 'characters/dots.png', None),
        'dots_speak': ('rgba', 'characters/dots.png', None),
    }

    total_bytes = 0
    count = 0

    for name, (stype, path, seeds) in sources.items():
        if stype == 'rgba':
            img = process_rgba(path)
        elif stype == 'white':
            img = isolate_white_bg(path, seed_points=seeds)
        elif stype == 'black':
            img = clean_black_bg(path)
        elif stype == 'scratch':
            img = Image.open(path).convert('RGB')
        else:
            raise ValueError(f'Unknown type: {stype}')

        # Resize to standard SPRITE_W x SPRITE_H
        sprite = img.resize((SPRITE_W, SPRITE_H), Image.Resampling.LANCZOS)
        sprite.save(os.path.join('scratch', 'sprites', f'{name}.png'))

        # Convert to RGB565 raw
        raw_data = to_rgb565(sprite)
        total_bytes += len(raw_data)
        count += 1

        # Save to spiffs_assets/anim/
        raw_path = os.path.join(spiffs_out, f'{name}.raw')
        with open(raw_path, 'wb') as f:
            f.write(raw_data)

        # Also copy crab to firmware/main for embedded fallback
        if name.startswith('crab_'):
            with open(os.path.join('firmware', 'main', f'{name}.raw'), 'wb') as f:
                f.write(raw_data)

        print(f'[{count:02d}/30] Generated {name:15s} -> {raw_path} ({len(raw_data)} bytes)')

    print('=' * 60)
    print(f'SUCCESS: Generated {count} animation frames in {spiffs_out}')
    print(f'Total binary payload: {total_bytes} bytes ({total_bytes / (1024*1024):.2f} MB)')
    print('=' * 60)

if __name__ == '__main__':
    main()
