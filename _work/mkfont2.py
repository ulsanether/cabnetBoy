import os, sys
from PIL import Image, ImageDraw, ImageFont

CHARS = "".join(chr(c) for c in range(32, 127))   # 95 glyphs


def raster(fontpath, px, thr=100, bold=False):
    f = ImageFont.truetype(fontpath, px)
    out = []
    for c in CHARS:
        pad = 12
        img = Image.new("L", (32, 32), 0)
        ImageDraw.Draw(img).text((pad, pad), c, font=f, fill=255)
        bbox = img.getbbox()
        if bbox is None:
            out.append([0] * 8)
            continue
        g = img.crop(bbox)
        if bold:
            g = g.point(lambda v: 255 if v >= thr else 0)
            g2 = Image.new("L", (g.width + 1, g.height), 0)
            g2.paste(g, (0, 0))
            g2.paste(g, (1, 0), g)
            g = g2
        # threshold
        g = g.point(lambda v: 255 if v >= thr else 0)
        w, h = g.size
        cell = Image.new("L", (8, 8), 0)
        if w > 8 or h > 8:
            # scale down preserving aspect
            s = min(8 / w, 8 / h)
            g = g.resize((max(1, int(w * s)), max(1, int(h * s))), Image.LANCZOS)
            g = g.point(lambda v: 255 if v >= 128 else 0)
            w, h = g.size
        x = (8 - w) // 2
        y = 8 - h          # baseline at bottom
        if y < 0:
            y = 0
        cell.paste(g, (x, y))
        rows = []
        for yy in range(8):
            b = 0
            for xx in range(8):
                if cell.getpixel((xx, yy)):
                    b |= 1 << (7 - xx)
            rows.append(b)
        out.append(rows)
    return out


def preview(glyphs, path, scale=6, cols=32):
    rows = (len(glyphs) + cols - 1) // cols
    img = Image.new("L", (cols * 9 * scale, rows * 9 * scale), 0)
    for i, g in enumerate(glyphs):
        cx = (i % cols) * 9 * scale
        cy = (i // cols) * 9 * scale
        for y in range(8):
            for x in range(8):
                if g[y] & (1 << (7 - x)):
                    img.paste(255, (cx + x * scale, cy + y * scale,
                                    cx + (x + 1) * scale, cy + (y + 1) * scale))
    img.save(path)
    return path


if __name__ == "__main__":
    for spec in [(r"C:\Windows\Fonts\lucon.ttf", 10, 100, False),
                 (r"C:\Windows\Fonts\lucon.ttf", 10, 100, True),
                 (r"C:\Windows\Fonts\consola.ttf", 10, 100, True),
                 (r"C:\Windows\Fonts\tahoma.ttf", 11, 100, True)]:
        g = raster(*spec)
        name = os.path.basename(spec[0]).split('.')[0] + f"_{spec[1]}_{'b' if spec[3] else 'n'}"
        preview(g, rf"D:\GIT\cabnetBoy\_work\fontprev_{name}.png")
        print("ok", name)
