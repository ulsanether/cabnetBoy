import os, sys
from PIL import Image, ImageDraw, ImageFont

# Rasterise an 8x8 cell font for ASCII 32..126 from a TTF.
FONTS = [r"C:\Windows\Fonts\lucon.ttf", r"C:\Windows\Fonts\consola.ttf", r"C:\Windows\Fonts\cour.ttf"]

def build(fontpath, px, ox, oy, thr=110):
    f = ImageFont.truetype(fontpath, px)
    glyphs = []
    for c in range(32, 127):
        img = Image.new("L", (8, 8), 0)
        d = ImageDraw.Draw(img)
        d.text((ox, oy), chr(c), font=f, fill=255)
        rows = []
        for y in range(8):
            b = 0
            for x in range(8):
                if img.getpixel((x, y)) >= thr:
                    b |= 1 << (7 - x)
            rows.append(b)
        glyphs.append(rows)
    return glyphs

def show(glyphs, chars=" ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.,:!?/-+()<>*#$%='\"[]_|@"):
    for ch in chars:
        i = ord(ch) - 32
        if not (0 <= i < len(glyphs)):
            continue
        print(f"--- '{ch}' ---")
        for r in glyphs[i]:
            print("".join("#" if r & (1 << (7 - x)) else "." for x in range(8)))

if __name__ == "__main__":
    src = sys.argv[1] if len(sys.argv) > 1 else FONTS[0]
    px = int(sys.argv[2]) if len(sys.argv) > 2 else 8
    ox = int(sys.argv[3]) if len(sys.argv) > 3 else 0
    oy = int(sys.argv[4]) if len(sys.argv) > 4 else 0
    thr = int(sys.argv[5]) if len(sys.argv) > 5 else 110
    g = build(src, px, ox, oy, thr)
    print("font:", src, "px:", px, "off:", ox, oy, "thr:", thr)
    show(g)
