#!/usr/bin/env python3
import os
import math
import numpy as np
from PIL import Image, ImageDraw, ImageFont, ImageFilter
from scipy.ndimage import label, binary_fill_holes, gaussian_filter

os.makedirs('assets/marketing', exist_ok=True)

# 4K UHD Dimensions
CANVAS_W, CANVAS_H = 3840, 2160

# Background: Walnut Shadow #100904
bg = Image.new('RGBA', (CANVAS_W, CANVAS_H), (16, 9, 4, 255))
draw = ImageDraw.Draw(bg)

# Draw subtle warm ambient darkroom glow in center
glow = Image.new('RGBA', (CANVAS_W, CANVAS_H), (0, 0, 0, 0))
glow_draw = ImageDraw.Draw(glow)
for r in range(1200, 200, -30):
    alpha = int(14 * (1.0 - (r / 1200.0)))
    glow_draw.ellipse(
        [CANVAS_W // 2 - r * 1.6, CANVAS_H // 2 - r * 0.85,
         CANVAS_W // 2 + r * 1.6, CANVAS_H // 2 + r * 0.85],
        fill=(56, 36, 22, alpha)
    )
glow = glow.filter(ImageFilter.GaussianBlur(80))
bg = Image.alpha_composite(bg, glow)

# Fonts
font_path_bold = 'C:/Windows/Fonts/segoeuib.ttf'
font_path_reg = 'C:/Windows/Fonts/segoeui.ttf'
font_path_disp = 'C:/Windows/Fonts/bahnschrift.ttf' if os.path.exists('C:/Windows/Fonts/bahnschrift.ttf') else font_path_bold

font_brand = ImageFont.truetype(font_path_disp, 52)
font_title = ImageFont.truetype(font_path_bold, 28)
font_sub = ImageFont.truetype(font_path_reg, 20)

font_char_name = ImageFont.truetype(font_path_bold, 36)
font_char_agent = ImageFont.truetype(font_path_bold, 22)
font_char_role = ImageFont.truetype(font_path_reg, 18)

draw = ImageDraw.Draw(bg)

# Top Header
brand_text = "KINPOD"
draw.text((CANVAS_W // 2, 140), brand_text, fill=(255, 237, 215), font=font_brand, anchor="mm")

title_text = "OFFICIAL COMPANION ROSTER"
draw.text((CANVAS_W // 2, 210), title_text, fill=(255, 237, 215), font=font_title, anchor="mm")

sub_text = "6 AUTONOMOUS AGENT EMBODIMENTS • 410×502 CURVED AMOLED • 60 FPS"
draw.text((CANVAS_W // 2, 260), sub_text, fill=(108, 95, 81), font=font_sub, anchor="mm")

# Hairline divider
draw.line([(300, 310), (CANVAS_W - 300, 310)], fill=(64, 55, 46), width=1)

def extract_foreground(image_path, bg_is_white=False):
    """Cleanly extract subject with soft feathered alpha."""
    im = Image.open(image_path).convert('RGB')
    arr = np.array(im, dtype=np.float32)
    h, w, _ = arr.shape
    r, g, b = arr[:, :, 0], arr[:, :, 1], arr[:, :, 2]
    
    if bg_is_white:
        is_bg = (r > 230) & (g > 230) & (b > 230)
    else:
        is_bg = (r < 20) & (g < 20) & (b < 20)
        
    labeled, _ = label(is_bg)
    seeds = [(0, 0), (0, w - 1), (h - 1, 0), (h - 1, w - 1)]
    seed_labels = set([labeled[sy, sx] for sy, sx in seeds if labeled[sy, sx] > 0])
    bg_mask = np.isin(labeled, list(seed_labels))
    fg_mask = binary_fill_holes(~bg_mask).astype(np.float32)
    
    # Smooth alpha feathering (2.0 px Gaussian blur)
    feathered_alpha = gaussian_filter(fg_mask, sigma=1.8)
    feathered_alpha = np.clip(feathered_alpha * 255.0, 0, 255).astype(np.uint8)
    
    im_rgba = im.convert('RGBA')
    im_rgba.putalpha(Image.fromarray(feathered_alpha))
    
    # Find bounding box
    bbox = im_rgba.split()[-1].getbbox()
    if bbox:
        # Add 4px padding so we don't clip any edge
        pad = 6
        x0 = max(0, bbox[0] - pad)
        y0 = max(0, bbox[1] - pad)
        x1 = min(w, bbox[2] + pad)
        y1 = min(h, bbox[3] + pad)
        im_rgba = im_rgba.crop((x0, y0, x1, y1))
        
    return im_rgba

ROSTER = [
    {
        'id': 'crab',
        'name': 'CRAB',
        'agent': 'OPENCLAW',
        'role': 'AUTONOMOUS DEV AGENT',
        'accent': (239, 68, 68),       # #EF4444
        'file': 'assets/marketing/characters/crab_uncropped.jpg',
        'is_white': False,
        'target_h': 600
    },
    {
        'id': 'astro',
        'name': 'ASTRO',
        'agent': 'GEMINI 2.5',
        'role': 'MULTIMODAL LIVE ASSISTANT',
        'accent': (255, 107, 0),      # #FF6B00
        'file': 'assets/marketing/characters/astro_uncropped.jpg',
        'is_white': False,
        'target_h': 650
    },
    {
        'id': 'octopus',
        'name': 'OCTO-CLAUDE',
        'agent': 'CLAUDE 3.7',
        'role': 'QUANTITATIVE ARCHITECT',
        'accent': (255, 122, 47),      # #FF7A2F
        'file': 'assets/marketing/characters/octopus_uncropped.jpg',
        'is_white': False,
        'target_h': 630
    },
    {
        'id': 'hermes',
        'name': 'HERMES',
        'agent': 'HERMES AGENT',
        'role': 'FUNCTION CALLER & ROUTER',
        'accent': (56, 189, 248),      # #38BDF8
        'file': 'assets/marketing/characters/hermes_uncropped.jpg',
        'is_white': False,
        'target_h': 650
    },
    {
        'id': 'muse',
        'name': 'MUSE',
        'agent': 'GPT-4O / CREATIVE',
        'role': 'VOICE & CREATIVE COMPANION',
        'accent': (251, 191, 36),      # #FBBF24
        'file': 'assets/marketing/characters/muse_uncropped.jpg',
        'is_white': False,
        'target_h': 640
    },
    {
        'id': 'dots',
        'name': 'DOTS',
        'agent': 'OLLAMA LOCAL',
        'role': '14B ON-DEVICE PRIVACY',
        'accent': (59, 130, 246),      # #3B82F6
        'file': 'assets/marketing/characters/dots_uncropped.jpg',
        'is_white': True,
        'target_h': 630
    }
]

num_chars = len(ROSTER)
col_width = (CANVAS_W - 200) / num_chars
char_baseline_y = 1380

for i, char in enumerate(ROSTER):
    cx = int(100 + col_width * (i + 0.5))
    
    # Extract uncropped subject
    subject = extract_foreground(char['file'], bg_is_white=char['is_white'])
    
    # Scale to target height
    scale = char['target_h'] / float(subject.height)
    new_w = int(subject.width * scale)
    new_h = int(subject.height * scale)
    scaled_char = subject.resize((new_w, new_h), Image.Resampling.LANCZOS)
    
    # Floor pedestal glow in accent color
    glow_spot = Image.new('RGBA', (CANVAS_W, CANVAS_H), (0, 0, 0, 0))
    glow_spot_draw = ImageDraw.Draw(glow_spot)
    cr, cg, cb = char['accent']
    glow_spot_draw.ellipse(
        [cx - 200, char_baseline_y - 45, cx + 200, char_baseline_y + 45],
        fill=(cr, cg, cb, 38)
    )
    glow_spot = glow_spot.filter(ImageFilter.GaussianBlur(38))
    bg = Image.alpha_composite(bg, glow_spot)
    
    # Paste character
    char_x = cx - new_w // 2
    char_y = char_baseline_y - new_h
    bg.paste(scaled_char, (char_x, char_y), scaled_char)
    
    # Text card beneath character
    info_y = 1460
    draw = ImageDraw.Draw(bg)
    
    # Accent indicator dot
    draw.ellipse([cx - 5, info_y - 25, cx + 5, info_y - 15], fill=char['accent'])
    
    # Name
    draw.text((cx, info_y + 10), char['name'], fill=(255, 237, 215), font=font_char_name, anchor="mm")
    
    # Agent affiliation
    draw.text((cx, info_y + 60), char['agent'], fill=char['accent'], font=font_char_agent, anchor="mm")
    
    # Role description
    draw.text((cx, info_y + 98), char['role'], fill=(108, 95, 81), font=font_char_role, anchor="mm")
    
    # Card outline frame
    card_w = int(col_width - 32)
    draw.rounded_rectangle(
        [cx - card_w // 2, info_y - 35, cx + card_w // 2, info_y + 130],
        radius=12,
        outline=(64, 55, 46),
        width=1
    )

# Footer bar
draw = ImageDraw.Draw(bg)
draw.line([(300, 1840), (CANVAS_W - 300, 1840)], fill=(64, 55, 46), width=1)
footer_callout = "TOUCH AMOLED SCREEN TO SWIPE BETWEEN AVATARS • CALL MCP 'kinpod_inhabit()' TO TAKE PHYSICAL POSSESSION"
draw.text((CANVAS_W // 2, 1920), footer_callout, fill=(255, 237, 215), font=font_sub, anchor="mm")
sub_footer = "COMPATIBLE WITH OPENCLAW • CLAUDE CODE • GEMINI • HERMES • OLLAMA LOCAL"
draw.text((CANVAS_W // 2, 1970), sub_footer, fill=(220, 80, 0), font=font_char_agent, anchor="mm")

# Save 4K JPG
final_out = bg.convert('RGB')
final_out.save('assets/marketing/characters_roster_4k.jpg', quality=95, subsampling=0)
print("Successfully generated uncropped 4K roster: assets/marketing/characters_roster_4k.jpg (3840x2160)")
