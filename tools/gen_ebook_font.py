#!/usr/bin/env python3
"""Generate native multi-size ebook fonts from LXGW WenKai (霞鹜文楷).

Each size is rasterized from the outline font (no bitmap stretching).
Sizes: 16, 20, 24 — chosen for novel reading on 320x240.
"""
from __future__ import annotations

import os
import sys
from PIL import Image, ImageDraw, ImageFont

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
OTF = os.path.join(ROOT, "tools", "fonts", "LXGWWenKai-Regular.ttf")
OUT_DIR = os.path.join(ROOT, "txt-reader", "main")
SIZES = (16, 20, 24)
FONT_LABEL = "WenKai"

# GBK novels often use Western curly quotes; draw CJK corner brackets instead
# so they stay full-width and readable at 16–24px.
GLYPH_DRAW_AS = {
    0x2018: "\u300c",  # ‘ → 「
    0x2019: "\u300d",  # ’ → 」
    0x201C: "\u300c",  # “ → 「
    0x201D: "\u300d",  # ” → 」
    0x201E: "\u300d",  # „ → 」
}


def keep_code(code: int) -> bool:
    if 32 <= code <= 126:
        return True
    if 160 <= code <= 255:
        return True
    # General Punctuation: “”‘’—… etc. (very common in GBK novels)
    if 0x2000 <= code <= 0x206F:
        return True
    if 0x2E80 <= code <= 0x2EFF:
        return True
    if 0x2F00 <= code <= 0x2FDF:
        return True
    if 0x3000 <= code <= 0x303F:
        return True
    # Skip Ext-A to keep 3 native sizes inside flash budget.
    if 0x4E00 <= code <= 0x9FFF:
        return True
    if 0xF900 <= code <= 0xFAFF:
        return True
    if 0xFE10 <= code <= 0xFE1F:
        return True
    if 0xFE30 <= code <= 0xFE4F:
        return True
    if 0xFF01 <= code <= 0xFF5E:
        return True
    return False


def is_whitespace_code(code: int) -> bool:
    """Codes that must keep advance width even with an empty bitmap."""
    if code in (0x20, 0xA0, 0x3000):
        return True
    if 0x2000 <= code <= 0x200B:  # en/em/thin spaces, ZWSP
        return True
    return False


def font_cmap(path: str) -> set[int]:
    from fontTools.ttLib import TTFont

    font = TTFont(path)
    codes: set[int] = set()
    for table in font["cmap"].tables:
        codes.update(table.cmap.keys())
    return codes


def find_bounding_box(image):
    pixels = image.load()
    width, height = image.size
    x_min, y_min = width, height
    x_max, y_max = 0, 0
    found = False
    for y in range(height):
        for x in range(width):
            if pixels[x, y] >= 1:
                found = True
                x_min = min(x_min, x)
                y_min = min(y_min, y)
                x_max = max(x_max, x)
                y_max = max(y_max, y)
    if not found:
        return None
    return (x_min, y_min, x_max + 1, y_max + 1)


def build_glyphs(pil_font, codes, font_size: int):
    glyphs = []
    skipped = 0
    canvas = font_size * 2 + 4
    for i, char_code in enumerate(codes):
        if i % 4000 == 0:
            print(f"    ... {i}/{len(codes)}", flush=True)
        char = chr(char_code)
        draw_char = GLYPH_DRAW_AS.get(char_code, char)
        # Grayscale then threshold → cleaner strokes than stretched pixel fonts
        gray = Image.new("L", (canvas, canvas), 0)
        draw = ImageDraw.Draw(gray)
        # slight baseline padding
        draw.text((2, 1), draw_char, font=pil_font, fill=255)
        # Slightly lower threshold so thin punctuation (“”「」) keeps ink
        bw = gray.point(lambda p: 1 if p >= 72 else 0, mode="1")
        bbox = find_bounding_box(bw)
        if bbox is None:
            if is_whitespace_code(char_code):
                try:
                    adv = int(round(draw.textlength(draw_char, font=pil_font)))
                except Exception:
                    adv = font_size if char_code >= 0x100 else max(1, font_size // 2)
                if char_code == 0x3000 or char_code >= 0x2000:
                    adv = max(adv, font_size)
                glyphs.append(
                    {
                        "char_code": char_code,
                        "ofs_y": 0,
                        "box_w": 0,
                        "box_h": 0,
                        "ofs_x": 0,
                        "adv_w": max(1, min(32, adv)),
                        "bitmap": [],
                    }
                )
            else:
                skipped += 1
            continue

        x0, y0, x1, y1 = bbox
        width, height = x1 - x0, y1 - y0
        offset_x, offset_y = x0 - 2, y0 - 1
        if offset_x < 0:
            offset_x = 0

        try:
            adv_w = int(round(draw.textlength(draw_char, font=pil_font)))
            adv_w = max(adv_w, width + offset_x)
        except Exception:
            adv_w = width + max(0, offset_x)
        # CJK-style quotes / punctuation should stay full-width
        if char_code in GLYPH_DRAW_AS or 0x3000 <= char_code <= 0x303F:
            adv_w = max(adv_w, font_size)

        if offset_y + height > font_size:
            if font_size - height >= 0:
                offset_y = font_size - height
            else:
                offset_y = 0
                height = font_size

        # Clamp to 32-bit row packing limit used by rg_gui
        if width > 32:
            width = 32
        if adv_w > 32:
            adv_w = 32

        cropped = bw.crop((x0, y0, x0 + width, y0 + height))
        bitmap = []
        row = 0
        bit_i = 0
        for y in range(height):
            for x in range(width):
                if bit_i == 8:
                    bitmap.append(row)
                    row = 0
                    bit_i = 0
                pixel = 1 if cropped.getpixel((x, y)) else 0
                row = (row << 1) | pixel
                bit_i += 1
        if bit_i:
            bitmap.append(row << (8 - bit_i))
        bitmap = bitmap[0 : (width * height + 7) // 8]

        glyphs.append(
            {
                "char_code": char_code,
                "ofs_y": int(max(0, offset_y)),
                "box_w": int(width),
                "box_h": int(height),
                "ofs_x": int(offset_x) & 0xFF,
                "adv_w": int(adv_w),
                "bitmap": bitmap,
            }
        )

    print(f"    glyphs={len(glyphs)} skipped={skipped}")
    return glyphs


def write_c(glyphs, font_size: int, symbol: str, out_path: str) -> int:
    max_height = max(font_size, max((g["ofs_y"] + g["box_h"]) for g in glyphs))
    blob = bytearray()
    offsets = []
    for g in glyphs:
        offsets.append(len(blob))
        code = g["char_code"]
        blob.extend(
            [
                code & 0xFF,
                (code >> 8) & 0xFF,
                g["ofs_y"] & 0xFF,
                g["box_w"] & 0xFF,
                g["box_h"] & 0xFF,
                g["ofs_x"] & 0xFF,
                g["adv_w"] & 0xFF,
            ]
        )
        blob.extend(g["bitmap"])
    blob.extend([0x00, 0x00, 0, 0, 0, 0, 0])
    memory = len(blob) + len(offsets) * 4

    with open(out_path, "w", encoding="utf-8", newline="\n") as f:
        f.write('#include "rg_gui.h"\n\n')
        f.write(f"// LXGW WenKai native {font_size}px — no stretch\n")
        f.write(f"// Memory usage   : {memory} bytes\n")
        f.write(f"// Characters     : {len(glyphs)}\n\n")
        f.write(f"static const uint32_t {symbol}_offsets[{len(offsets)}] = {{\n")
        for i in range(0, len(offsets), 12):
            f.write("    " + ", ".join(str(x) for x in offsets[i : i + 12]) + ",\n")
        f.write("};\n\n")
        f.write(f"const rg_font_t {symbol} = {{\n")
        f.write(f'    .name = "{FONT_LABEL} {font_size}",\n')
        f.write("    .type = 1,\n")
        f.write("    .width = 0,\n")
        f.write(f"    .height = {max_height},\n")
        f.write(f"    .chars = {len(glyphs)},\n")
        f.write(f"    .offsets = {symbol}_offsets,\n")
        f.write("    .data = {\n")
        for i in range(0, len(blob), 16):
            f.write("        " + ", ".join(f"0x{b:02X}" for b in blob[i : i + 16]) + ",\n")
        f.write("    },\n};\n")
    print(f"  Wrote {out_path} flash~{memory}")
    return memory


def main():
    if not os.path.exists(OTF) or os.path.getsize(OTF) < 1000000:
        print(f"Missing/invalid font: {OTF}", file=sys.stderr)
        return 1

    codes = sorted(c for c in font_cmap(OTF) if keep_code(c))
    print(f"WenKai codes kept: {len(codes)}")
    for must in (0x6D52, ord("水"), ord("传"), ord("国"), 0x201C, 0x201D, 0x300C, 0x300D, 0x3000):
        print(f"  U+{must:04X} {'OK' if must in set(codes) else 'MISSING'}")

    mapping = {
        16: ("font_ebook", "font_ebook.c"),
        20: ("font_ebook_20", "font_ebook_20.c"),
        24: ("font_ebook_24", "font_ebook_24.c"),
    }
    total = 0
    for size in SIZES:
        print(f"Building native {size}px...")
        pil = ImageFont.truetype(OTF, size)
        glyphs = build_glyphs(pil, codes, size)
        symbol, name = mapping[size]
        total += write_c(glyphs, size, symbol, os.path.join(OUT_DIR, name))

    print(f"All done. Combined flash ~{total} bytes")
    if total > 2200000:
        print("WARNING: fonts may exceed txt-reader partition; reduce charset/sizes", file=sys.stderr)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
