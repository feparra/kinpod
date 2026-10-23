#!/usr/bin/env python3
import os
from PIL import Image, ImageDraw, ImageFont
import numpy as np

os.makedirs('firmware/main/assets', exist_ok=True)

font_title = ImageFont.truetype('C:/Windows/Fonts/segoeuib.ttf', 24)
font_sub = ImageFont.truetype('C:/Windows/Fonts/segoeui.ttf', 13)
font_status = ImageFont.truetype('C:/Windows/Fonts/segoeuib.ttf', 12)
font_arrow = ImageFont.truetype('C:/Windows/Fonts/segoeui.ttf', 12)

ROSTER = [
    {
        'id': 'astro',
        'name': 'GEMINI',
        'sub': 'MULTIMODAL LIVE',
        'accent': (255, 107, 0),      # #FF6B00
        'img': 'characters/astro.png'
    },
    {
        'id': 'crab',
        'name': 'OPENCLAW',
        'sub': 'AUTONOMOUS AGENT',
        'accent': (239, 68, 68),       # #EF4444
        'img': 'characters/crab.png'
    },
    {
        'id': 'hermes',
        'name': 'HERMES',
        'sub': 'FUNCTION CALLER',
        'accent': (56, 189, 248),      # #38BDF8
        'img': 'characters/hermes.png'
    },
    {
        'id': 'muse',
        'name': 'MUSE',
        'sub': 'CREATIVE ASSISTANT',
        'accent': (251, 191, 36),      # #FBBF24
        'img': 'characters/muse.png'
    },
    {
        'id': 'octopus',
        'name': 'OCTO-CLAUDE',
        'sub': 'QUANTITATIVE AGENT',
        'accent': (255, 122, 47),      # #FF7A2F
        'img': 'characters/octopus.png'
    },
    {
        'id': 'dots',
        'name': 'DOTS LOCAL',
        'sub': 'OLLAMA CODER 14B',
        'accent': (59, 130, 246),      # #3B82F6
        'img': 'characters/dots.png'
    }
]

W, H = 410, 502

for idx, r in enumerate(ROSTER):
    img = Image.new('RGB', (W, H), (0, 0, 0))
    draw = ImageDraw.Draw(img)

    accent = r['accent']
    char_id = r['id']

    # 1. Top Bar: Battery & Status (Positioned inside Safe Area away from rounded corners)
    # Status pill: 'ONLINE' with glowing dot (x=38, y=26)
    pill_w = 88
    pill_h = 24
    pill_x = 38
    pill_y = 26
    draw.rounded_rectangle([pill_x, pill_y, pill_x + pill_w, pill_y + pill_h], radius=12, fill=(18, 18, 20), outline=accent, width=1)
    draw.ellipse([pill_x + 8, pill_y + 7, pill_x + 18, pill_y + 17], fill=accent)
    draw.text((pill_x + 24, pill_y + 4), 'ONLINE', fill=(240, 240, 240), font=font_status)

    # Battery icon outline & charge indicator (x=W-82, y=28)
    bat_x, bat_y = W - 82, 28
    draw.rounded_rectangle([bat_x, bat_y, bat_x + 36, bat_y + 18], radius=4, outline=(180, 180, 180), width=2)
    draw.rectangle([bat_x + 36, bat_y + 5, bat_x + 39, bat_y + 13], fill=(180, 180, 180)) # battery terminal tip
    draw.rounded_rectangle([bat_x + 3, bat_y + 3, bat_x + 27, bat_y + 15], radius=2, fill=(40, 220, 70)) # 85% charge level

    # 2. Character Name & Subtitle
    name_w = draw.textlength(r['name'], font=font_title)
    draw.text(((W - name_w) / 2, 64), r['name'], fill=(255, 255, 255), font=font_title)

    sub_w = draw.textlength(r['sub'], font=font_sub)
    draw.text(((W - sub_w) / 2, 94), r['sub'], fill=accent, font=font_sub)

    # 3. Paste Character Sprite (Card removed for pure AMOLED black)
    char_im = Image.open(r['img']).convert('RGBA')
    sprite_size = 240
    char_resized = char_im.resize((sprite_size, sprite_size), Image.Resampling.LANCZOS)
    sp_x = (W - sprite_size) // 2
    sp_y = 130
    img.paste(char_resized, (sp_x, sp_y), char_resized)

    # 5. Bottom Navigation: Dots Indicator (6 dots)
    dot_spacing = 22
    total_dots_w = 5 * dot_spacing
    start_dot_x = (W - total_dots_w) // 2
    dots_y = 398

    for d in range(6):
        dx = start_dot_x + d * dot_spacing
        if d == idx:
            draw.ellipse([dx - 5, dots_y - 5, dx + 5, dots_y + 5], fill=accent)
        else:
            draw.ellipse([dx - 3, dots_y - 3, dx + 3, dots_y + 3], fill=(70, 70, 70))

    # Swipe text with arrows
    hint = '◄   SWIPE TO SWITCH   ►'
    hint_w = draw.textlength(hint, font=font_arrow)
    draw.text(((W - hint_w) / 2, 432), hint, fill=(120, 120, 120), font=font_arrow)

    # Save PNG mockup
    png_out = os.path.join('firmware', 'main', 'assets', f'avatar_{char_id}.png')
    img.save(png_out)

    # Convert to RGB565 raw format
    raw = bytearray()
    for y in range(H):
        for x in range(W):
            pr, pg, pb = img.getpixel((x, y))
            rgb565 = ((pr & 0xF8) << 8) | ((pg & 0xFC) << 3) | (pb >> 3)
            raw.append((rgb565 >> 8) & 0xFF)
            raw.append(rgb565 & 0xFF)

    raw_out = os.path.join('firmware', 'main', 'assets', f'avatar_{char_id}.raw')
    with open(raw_out, 'wb') as f:
        f.write(raw)

    raw_main = os.path.join('firmware', 'main', f'avatar_{char_id}.raw')
    with open(raw_main, 'wb') as f:
        f.write(raw)

    print(f'Generated {char_id:8s} -> {raw_out} and {raw_main} ({len(raw)} bytes)')

print('All 6 avatar screens generated successfully!')
