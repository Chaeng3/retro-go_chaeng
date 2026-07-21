#!/usr/bin/env python3
"""Regenerate launcher UI font (FusionPixel12) with full GB2312 coverage.

The stock subset (~3950 glyphs) misses characters like 浒 (U+6D52) that appear
in SD book filenames. This rebuilds from the Fusion Pixel 12px OTF.
"""
from __future__ import annotations

import os
import sys
from PIL import Image, ImageDraw, ImageFont

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
OTF = os.path.join(ROOT, "tools", "fonts", "fusion-pixel-12px-monospaced-zh_hans.otf")
OUT = os.path.join(ROOT, "components", "retro-go", "fonts", "FusionPixel12.c")
FONT_SIZE = 12


def font_cmap(path: str) -> set[int]:
    from fontTools.ttLib import TTFont

    font = TTFont(path)
    codes: set[int] = set()
    for table in font["cmap"].tables:
        codes.update(table.cmap.keys())
    return codes


def gb2312_codes() -> set[int]:
    codes: set[int] = set()
    for lead in range(0xA1, 0xF8):
        for trail in range(0xA1, 0xFF):
            try:
                ch = bytes((lead, trail)).decode("gb2312")
            except UnicodeDecodeError:
                continue
            if len(ch) == 1:
                codes.add(ord(ch))
    return codes


def keep_code(code: int, available: set[int], gb: set[int]) -> bool:
    if code not in available:
        return False
    if 32 <= code <= 126:
        return True
    if 160 <= code <= 255:
        return True
    if 0x2000 <= code <= 0x206F:
        return True
    if 0x3000 <= code <= 0x303F:
        return True
    if 0xFF01 <= code <= 0xFF5E:
        return True
    if code in gb:
        return True
    return False


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


def build_glyphs(pil_font, codes):
    glyphs = []
    skipped = 0
    canvas = FONT_SIZE * 2
    for i, char_code in enumerate(codes):
        if i % 2000 == 0:
            print(f"  ... {i}/{len(codes)}", flush=True)
        char = chr(char_code)
        image = Image.new("1", (canvas, canvas), 0)
        draw = ImageDraw.Draw(image)
        draw.text((1, 0), char, font=pil_font, fill=255)
        bbox = find_bounding_box(image)
        if bbox is None:
            if char_code in (0x20, 0xA0, 0x3000) or 0x2000 <= char_code <= 0x200B:
                try:
                    adv = int(round(draw.textlength(char, font=pil_font)))
                except Exception:
                    adv = FONT_SIZE if char_code >= 0x100 else 6
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
        offset_x, offset_y = x0, y0
        if offset_x:
            offset_x -= 1
        if offset_x < 0:
            offset_x = 0

        try:
            adv_w = int(round(draw.textlength(char, font=pil_font)))
            adv_w = max(adv_w, width + offset_x)
        except Exception:
            adv_w = width + offset_x

        if offset_y + height > FONT_SIZE:
            if FONT_SIZE - height >= 0:
                offset_y = FONT_SIZE - height
            else:
                offset_y = 0
                height = FONT_SIZE

        if width > 32:
            width = 32
        if adv_w > 32:
            adv_w = 32

        cropped = image.crop((x0, y0, x0 + width, y0 + height))
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

    print(f"  glyphs={len(glyphs)} skipped={skipped}")
    return glyphs


def write_c(glyphs, out_path: str) -> int:
    max_height = max(FONT_SIZE, max((g["ofs_y"] + g["box_h"]) for g in glyphs))
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
        f.write('#include "../rg_gui.h"\n\n')
        f.write("// File generated with tools/gen_fusion_ui_font.py\n\n")
        f.write("// Font           : Fusion Pixel 12px M zh_hans Regular\n")
        f.write(f"// Point Size     : {FONT_SIZE}\n")
        f.write(f"// Memory usage   : {memory} bytes\n")
        f.write(f"// Characters     : {len(glyphs)} (GB2312 + ASCII/punct)\n\n")
        f.write(f"static const uint32_t font_FusionPixel_offsets[{len(offsets)}] = {{\n")
        for i in range(0, len(offsets), 12):
            f.write("    " + ", ".join(str(x) for x in offsets[i : i + 12]) + ",\n")
        f.write("};\n\n")
        f.write("const rg_font_t font_FusionPixel = {\n")
        f.write('    .name = "FusionPixel 12",\n')
        f.write("    .type = 1,\n")
        f.write("    .width = 0,\n")
        f.write(f"    .height = {max_height},\n")
        f.write(f"    .chars = {len(glyphs)},\n")
        f.write("    .offsets = font_FusionPixel_offsets,\n")
        f.write("    .data = {\n")
        for i in range(0, len(blob), 16):
            f.write("        " + ", ".join(f"0x{b:02X}" for b in blob[i : i + 16]) + ",\n")
        f.write("    },\n};\n")
    print(f"Wrote {out_path} flash~{memory}")
    return memory


def main():
    if not os.path.exists(OTF):
        print(f"Missing font: {OTF}", file=sys.stderr)
        return 1

    available = font_cmap(OTF)
    gb = gb2312_codes()
    codes = sorted(c for c in available if keep_code(c, available, gb))
    print(f"Available={len(available)} GB2312={len(gb)} kept={len(codes)}")
    for must in (0x6D52, ord("水"), ord("传"), ord("国"), ord("红"), ord("楼")):
        print(f"  U+{must:04X} {'OK' if must in set(codes) else 'MISSING'}")

    pil = ImageFont.truetype(OTF, FONT_SIZE)
    glyphs = build_glyphs(pil, codes)
    mem = write_c(glyphs, OUT)
    if mem > 250000:
        print("WARNING: UI font got large; launcher partition may need bump", file=sys.stderr)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
