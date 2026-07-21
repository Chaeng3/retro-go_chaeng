#!/usr/bin/env python3
"""Generate logo/banner/background assets for the txt (E-Book) tab.

Style matches other default themes: solid-ish background, light header logo,
magenta-transparent banner text.
"""
from PIL import Image, ImageDraw
import os

base = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "themes", "default"))

# --- logo_txt 46x50: open book on light header bg (like logo_nes) ---
bg = (248, 252, 248, 255)
cover = (120, 72, 40, 255)
cover_lt = (168, 112, 64, 255)
cover_dk = (72, 40, 20, 255)
paper = (255, 244, 220, 255)
paper2 = (248, 232, 200, 255)
ink = (96, 72, 48, 255)
ribbon = (200, 48, 48, 255)

logo = Image.new("RGBA", (46, 50), bg)
d = ImageDraw.Draw(logo)
# Book body
d.rounded_rectangle([8, 10, 38, 42], radius=2, fill=cover, outline=cover_dk)
# Open pages
d.polygon([(10, 14), (23, 12), (23, 40), (11, 41)], fill=paper, outline=cover_dk)
d.polygon([(23, 12), (36, 14), (35, 41), (23, 40)], fill=paper2, outline=cover_dk)
d.line([(23, 12), (23, 40)], fill=cover_dk, width=1)
# Text lines
for y in (17, 21, 25, 29, 33):
    d.line([(13, y), (21, y)], fill=ink, width=1)
    d.line([(25, y), (33, y)], fill=ink, width=1)
# Bookmark ribbon
d.polygon([(21, 10), (25, 10), (23, 20)], fill=ribbon)
# Soft highlight on cover edge
d.line([(9, 12), (9, 40)], fill=cover_lt, width=1)
logo.convert("P", palette=Image.ADAPTIVE, colors=32).save(
    os.path.join(base, "logo_txt.png"), optimize=True
)

# --- banner_txt 272x24 ---
fonts = {
    "E": ["11111", "10000", "11110", "10000", "11111"],
    "B": ["11110", "10001", "11110", "10001", "11110"],
    "O": ["01110", "10001", "10001", "10001", "01110"],
    "K": ["10001", "10010", "11100", "10010", "10001"],
    "-": ["00000", "00000", "11111", "00000", "00000"],
}


def draw_char(draw, ch, x, y, color, s=2):
    rows = fonts.get(ch)
    if not rows:
        return 6 * s
    for r, row in enumerate(rows):
        for c, bit in enumerate(row):
            if bit == "1":
                draw.rectangle(
                    [x + c * s, y + r * s, x + c * s + s - 1, y + r * s + s - 1],
                    fill=color,
                )
    return len(rows[0]) * s + s


banner = Image.new("RGBA", (272, 24), (255, 0, 255, 255))
d = ImageDraw.Draw(banner)
x = 4
for ch in "E-BOOK":
    x += draw_char(d, ch, x, 7, (248, 252, 248, 255), 2)

banner2 = Image.new("P", (272, 24))
banner2.putpalette([255, 0, 255, 248, 252, 248] + [0] * 750)
src, dst = banner.load(), banner2.load()
for yy in range(24):
    for xx in range(272):
        r, g, b, _ = src[xx, yy]
        dst[xx, yy] = 0 if (r, g, b) == (255, 0, 255) else 1
banner2.save(os.path.join(base, "banner_txt.png"), optimize=True)

# --- background_txt 320x240: warm leather/parchment like other solid themes ---
# Dominant warm brown (similar density to NES red / GB orange themes)
bgim = Image.new("RGB", (320, 240), (148, 92, 48))
d = ImageDraw.Draw(bgim)

# Two-tone base like NES/GB backgrounds
for y in range(240):
    shade = 148 if (y // 16) % 2 == 0 else 132
    d.line([(0, y), (319, y)], fill=(shade, shade - 52, shade - 96))

# Large open-book silhouette (center)
# Left cover
d.polygon([(48, 50), (150, 40), (150, 200), (56, 208)], fill=(112, 64, 32), outline=(72, 40, 16))
# Right cover
d.polygon([(170, 40), (272, 50), (264, 208), (170, 200)], fill=(120, 72, 36), outline=(72, 40, 16))
# Pages
d.polygon([(70, 58), (150, 50), (150, 192), (76, 198)], fill=(236, 220, 188), outline=(96, 64, 32))
d.polygon([(170, 50), (250, 58), (244, 198), (170, 192)], fill=(228, 212, 176), outline=(96, 64, 32))
# Spine gap
d.rectangle([150, 42, 170, 198], fill=(88, 48, 24))
# Page lines
for y in range(70, 180, 10):
    d.line([(84, y), (140, y)], fill=(180, 150, 110), width=1)
    d.line([(180, y), (236, y)], fill=(180, 150, 110), width=1)
# Ribbon
d.polygon([(152, 42), (168, 42), (160, 90)], fill=(192, 48, 40))

# Soft vignette
for i in range(24):
    c = 148 - i * 2
    d.rectangle([i, i, 319 - i, 239 - i], outline=(max(80, c), max(40, c - 52), max(20, c - 96)))

bgim.convert("P", palette=Image.ADAPTIVE, colors=64).save(
    os.path.join(base, "background_txt.png"), optimize=True
)

for name in ("logo_txt.png", "banner_txt.png", "background_txt.png"):
    path = os.path.join(base, name)
    print(name, os.path.getsize(path), "bytes")
