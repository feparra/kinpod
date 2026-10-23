#!/usr/bin/env python3
import os
import numpy as np
from PIL import Image
from scipy.ndimage import label, binary_fill_holes, distance_transform_edt

SPRITE_W = 240
SPRITE_H = 240

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

def clean_black_bg(img_path, threshold=25):
    im = Image.open(img_path).convert('RGB')
    arr = np.array(im)
    is_dark = (arr[:, :, 0] < threshold) & (arr[:, :, 1] < threshold) & (arr[:, :, 2] < threshold)
    arr[is_dark] = 0
    return Image.fromarray(arr, mode='RGB')

def main():
    os.makedirs('firmware/main', exist_ok=True)
    os.makedirs('scratch/sprites', exist_ok=True)

    sources = {
        'crab_idle': {
            'type': 'rgba_file',
            'path': 'characters/crab.png'
        },
        'crab_blink': {
            'type': 'white_bg',
            'path': r'C:\Users\FERNA\.gemini\antigravity-ide\brain\b98c037a-71c2-4c3e-88c5-6eb9bd470865\crab_blink_frame_1792613449796.jpg'
        },
        'crab_listen': {
            'type': 'white_bg',
            'path': r'C:\Users\FERNA\.gemini\antigravity-ide\brain\b98c037a-71c2-4c3e-88c5-6eb9bd470865\crab_listening_frame_1792613325825.jpg',
            'seeds': [(int(0.78 * 1024), 400)]
        },
        'crab_think': {
            'type': 'black_bg',
            'path': r'C:\Users\FERNA\.gemini\antigravity-ide\brain\b98c037a-71c2-4c3e-88c5-6eb9bd470865\crab_thinking_frame_1792613434497.jpg'
        },
        'crab_speak': {
            'type': 'white_bg',
            'path': r'C:\Users\FERNA\.gemini\antigravity-ide\brain\b98c037a-71c2-4c3e-88c5-6eb9bd470865\crab_speaking_frame_1792613308919.jpg'
        }
    }

    for name, cfg in sources.items():
        stype = cfg['type']
        path = cfg['path']
        if stype == 'rgba_file':
            im_src = Image.open(path).convert('RGBA')
            # Paste onto pure black
            base = Image.new('RGB', (im_src.width, im_src.height), (0, 0, 0))
            base.paste(im_src, (0, 0), im_src)
            cleaned = base
        elif stype == 'white_bg':
            seeds = cfg.get('seeds', None)
            cleaned = isolate_white_bg(path, seed_points=seeds)
        elif stype == 'black_bg':
            cleaned = clean_black_bg(path)

        # Resize to standard SPRITE_W x SPRITE_H
        sprite = cleaned.resize((SPRITE_W, SPRITE_H), Image.Resampling.LANCZOS)
        
        # Save PNG for inspection
        png_path = os.path.join('scratch', 'sprites', f'{name}.png')
        sprite.save(png_path)

        # Convert to RGB565 raw
        raw_data = to_rgb565(sprite)
        
        # Save raw to firmware/main for embedding in flash
        raw_firmware_path = os.path.join('firmware', 'main', f'{name}.raw')
        with open(raw_firmware_path, 'wb') as f:
            f.write(raw_data)

        print(f'Generated {name:12s} -> {raw_firmware_path} ({len(raw_data)} bytes)')

    print(f'All 5 sprites generated successfully at {SPRITE_W}x{SPRITE_H} RGB565!')

if __name__ == '__main__':
    main()
