import sys
from PIL import Image

BOUNDS = (-10.0, -62.5, 42.0, 3.5)
MARGIN = 0.5
DPI = 900
PPM = DPI / 25.4
X0 = BOUNDS[0] - MARGIN
Y1 = BOUNDS[3] + MARGIN
W = (BOUNDS[2] - BOUNDS[0]) + 2 * MARGIN
H = (BOUNDS[1] * -1 + BOUNDS[3]) + 2 * MARGIN

def to_px(x, y):
    return int((x - X0) * PPM), int((Y1 - y) * PPM)

def crop(img_path, out, xa, ya, xb, yb, zoom=2):
    im = Image.open(img_path).convert("RGB")
    px0, py0 = to_px(xa, ya)
    px1, py1 = to_px(xb, yb)
    px0, px1 = max(0, min(px0, px1)), min(im.width, max(px0, px1))
    py0, py1 = max(0, min(py0, py1)), min(im.height, max(py0, py1))
    c = im.crop((px0, py0, px1, py1))
    if zoom != 1:
        c = c.resize((int(c.width * zoom), int(c.height * zoom)), Image.LANCZOS)
    c.save(out)
    print(out, c.size)

if __name__ == "__main__":
    top = r"D:\GIT\cabnetBoy\_work\board_top.png"
    bot = r"D:\GIT\cabnetBoy\_work\board_bottom.png"
    o = r"D:\GIT\cabnetBoy\_work\crops"
    crop(top, o + r"\z1_headers.png", -2, 4, 22, -4, 3)
    crop(top, o + r"\z2_slots.png", 13, -45, 30, -54, 2.5)
    crop(bot, o + r"\z3_clearance.png", 20, -6, 32, -14, 3)
    crop(bot, o + r"\z4_silkbottom.png", -1, -32, 8, -43, 3)
    crop(bot, o + r"\z5_u5area.png", 20, -4, 34, -20, 2.5)
    crop(top, o + r"\z6_power.png", -9, 3, 4, -10, 3)
