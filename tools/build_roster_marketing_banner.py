#!/usr/bin/env python3
import os
import math
from PIL import Image, ImageDraw, ImageFont, ImageFilter

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
    alpha = int(12 * (1.0 - (r / 1200.0)))
    glow_draw.ellipse(
        [CANVAS_W // 2 - r * 1.5, CANVAS_H // 2 - r * 0.8,
         CANVAS_W // 2 + r * 1.5, CANVAS_H // 2 + r * 0.8],
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
font_badge = ImageFont.truetype(font_path_bold, 14)

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

# The 6 Real Kin Characters
ROSTER = [
    {
        'id': 'crab',
        'name': 'CRAB',
        'agent': 'OPENCLAW',
        'role': 'AUTONOMOUS DEV AGENT',
        'accent': (239, 68, 68),       # #EF4444
        'raw': 'characters/crab_raw.png',
        'clean': 'characters/crab.png',
        'target_h': 580
    },
    {
        'id': 'astro',
        'name': 'ASTRO',
        'agent': 'GEMINI 2.5',
        'role': 'MULTIMODAL LIVE ASSISTANT',
        'accent': (255, 107, 0),      # #FF6B00
        'raw': 'characters/astro_raw.png',
        'clean': 'characters/astro.png',
        'target_h': 640
    },
    {
        'id': 'octopus',
        'name': 'OCTO-CLAUDE',
        'agent': 'CLAUDE 3.7',
        'role': 'QUANTITATIVE ARCHITECT',
        'accent': (255, 122, 47),      # #FF7A2F
        'raw': 'characters/octopus_raw.png',
        'clean': 'characters/octopus.png',
        'target_h': 640
    },
    {
        'id': 'hermes',
        'name': 'HERMES',
        'agent': 'HERMES AGENT',
        'role': 'FUNCTION CALLER & ROUTER',
        'accent': (56, 189, 248),      # #38BDF8
        'raw': 'characters/hermes_raw.png',
        'clean': 'characters/hermes.png',
        'target_h': 650
    },
    {
        'id': 'muse',
        'name': 'MUSE',
        'agent': 'GPT-4O / CREATIVE',
        'role': 'VOICE & CREATIVE COMPANION',
        'accent': (251, 191, 36),      # #FBBF24
        'raw': 'characters/muse_raw.png',
        'clean': 'characters/muse.png',
        'target_h': 640
    },
    {
        'id': 'dots',
        'name': 'DOTS',
        'agent': 'OLLAMA LOCAL',
        'role': '14B ON-DEVICE PRIVACY',
        'accent': (59, 130, 246),      # #3B82F6
        'raw': 'characters/dots_raw.png',
        'clean': 'characters/dots.png',
        'target_h': 640
    }
]

# Calculate layout across 6 columns
num_chars = len(ROSTER)
col_width = (CANVAS_W - 200) / num_chars
char_baseline_y = 1380

for i, char in enumerate(ROSTER):
    cx = int(100 + col_width * (i + 0.5))
    
    # Process high-res character image
    raw_img = Image.open(char['raw']).convert('RGBA')
    
    # Extract tight bounding box of character
    gray = raw_img.convert('L')
    bbox = gray.point(lambda p: 255 if p > 12 else 0).getbbox()
    if bbox:
        cropped = raw_img.crop(bbox)
    else:
        cropped = raw_img
    
    # Create smooth alpha channel keyed from background black
    r, g, b, a = cropped.split()
    # Compute luminance mask to key out pure black smoothly
    luminance = cropped.convert('L')
    alpha = luminance.point(lambda p: min(255, int(p * 2.8)) if p > 5 else 0)
    cropped.putalpha(alpha)
    
    # Scale keeping aspect ratio
    scale = char['target_h'] / float(cropped.height)
    new_w = int(cropped.width * scale)
    new_h = int(cropped.height * scale)
    scaled_char = cropped.resize((new_w, new_h), Image.Resampling.LANCZOS)
    
    # Draw floor glow ellipse in character accent color
    glow_spot = Image.new('RGBA', (CANVAS_W, CANVAS_H), (0, 0, 0, 0))
    glow_spot_draw = ImageDraw.Draw(glow_spot)
    cr, cg, cb = char['accent']
    glow_spot_draw.ellipse(
        [cx - 180, char_baseline_y - 40, cx + 180, char_baseline_y + 40],
        fill=(cr, cg, cb, 30)
    )
    glow_spot = glow_spot.filter(ImageFilter.GaussianBlur(35))
    bg = Image.alpha_composite(bg, glow_spot)
    
    # Composite character onto canvas
    char_x = cx - new_w // 2
    char_y = char_baseline_y - new_h
    bg.paste(scaled_char, (char_x, char_y), scaled_char)
    
    # Draw card info beneath character
    info_y = 1460
    
    # Accent indicator dot
    draw = ImageDraw.Draw(bg)
    draw.ellipse([cx - 5, info_y - 25, cx + 5, info_y - 15], fill=char['accent'])
    
    # Name (CRAB, ASTRO, etc.)
    draw.text((cx, info_y + 10), char['name'], fill=(255, 237, 215), font=font_char_name, anchor="mm")
    
    # Agent affiliation (OPENCLAW, GEMINI, etc.)
    draw.text((cx, info_y + 60), char['agent'], fill=char['accent'], font=font_char_agent, anchor="mm")
    
    # Role description
    draw.text((cx, info_y + 98), char['role'], fill=(108, 95, 81), font=font_char_role, anchor="mm")
    
    # Subtle frame card around text
    card_w = int(col_width - 30)
    draw.rounded_rectangle(
        [cx - card_w // 2, info_y - 35, cx + card_w // 2, info_y + 130],
        radius=12,
        outline=(64, 55, 46),
        width=1
    )

# Bottom bar: Hardware capability callout
draw = ImageDraw.Draw(bg)
draw.line([(300, 1840), (CANVAS_W - 300, 1840)], fill=(64, 55, 46), width=1)
footer_callout = "TOUCH AMOLED SCREEN TO SWIPE BETWEEN AVATARS • CALL MCP 'kinpod_inhabit()' TO TAKE PHYSICAL POSSESSION"
draw.text((CANVAS_W // 2, 1920), footer_callout, fill=(255, 237, 215), font=font_sub, anchor="mm")
sub_footer = "COMPATIBLE WITH OPENCLAW • CLAUDE CODE • GEMINI • HERMES • OLLAMA LOCAL"
draw.text((CANVAS_W // 2, 1970), sub_footer, fill=(220, 80, 0), font=font_char_agent, anchor="mm")

# Convert to RGB and save in highest 4K JPEG quality
final_out = bg.convert('RGB')
final_out.save('assets/marketing/characters_roster_4k.jpg', quality=95, subsampling=0)
print("Successfully generated assets/marketing/characters_roster_4k.jpg (3840x2160)")
