"""Host preview of the cabnetBoy v2 self-test screen.

Parses font8.h / tiles.h straight out of software/cabnetBoyTest/src and mirrors
draw() from test.c so the layout can be checked without the board.
"""
import re, os, sys
from PIL import Image

SRC = r"D:\GIT\cabnetBoy\software\cabnetBoyTest\src"
VIEW_W = 16

def _hexlist(path, marker):
    txt = open(path, encoding="utf-8").read()
    body = txt.split("= {", 1)[1].split("};", 1)[0]
    body = re.sub(r"//[^\n]*", "", body)
    return [int(v, 16) for v in re.findall(r"0x([0-9A-Fa-f]{2})", body)]

FONT = _hexlist(os.path.join(SRC, "font8.h"), "FONT8")
TILES_ = _hexlist(os.path.join(SRC, "tiles.h"), "TILES")
TILES = [TILES_[i*8:(i+1)*8] for i in range(len(TILES_) // 8)]

G_VOID, G_FULL, G_HALF, G_BOX = 0, 1, 2, 3

def glyph(value):
    if value < 32:
        return TILES[value] if value < len(TILES) else TILES[G_VOID]
    if value > 95:
        value = 32
    i = (value - 32) * 8
    return FONT[i:i + 8]

def render(rows, btn_inv, bar_level):
    img = Image.new("1", (128, 64), 0)
    px = img.load()
    for page in range(8):
        cells = [0] * VIEW_W
        inv = [False] * VIEW_W
        if page == 4:
            # gfx_bar: level 0..255 -> 16 half cells over 8 cells
            filled = (bar_level * 16) >> 8
            for i in range(8):
                if filled >= 2:
                    cells[i] = G_FULL; filled -= 2
                elif filled == 1:
                    cells[i] = G_HALF; filled = 0
                else:
                    cells[i] = G_VOID
        elif page == 1:
            # mirrors test.c: blank row, then U D L R A S at columns 2,4,6,8,10,12
            lbl = "UDLRAS"
            for i in range(VIEW_W):
                cells[i] = ord(' ')
            for i in range(6):
                cells[2 + i * 2] = ord(lbl[i])
                inv[2 + i * 2] = bool(btn_inv[i])
        else:
            for i, ch in enumerate(rows[page][:VIEW_W]):
                cells[i] = ord(ch)
        for col, val in enumerate(cells):
            b = glyph(val)
            for x in range(8):
                for y in range(8):
                    lit = bool(b[y] & (0x80 >> x))
                    if inv[col]:
                        lit = not lit
                    if lit:
                        px[col * 8 + x, page * 8 + y] = 1
    return img

def make(state):
    rows = [" " * VIEW_W for _ in range(8)]
    def setrow(r, s):
        s = s.upper()
        rows[r] = (s + " " * VIEW_W)[:VIEW_W]
    detents, a, b = state["enc"]
    setrow(0, "CABNETBOY V2")
    rows[2] = list("ENC +000 AB00")
    rows[2][4:8] = list("%+04d" % detents)
    rows[2][11] = "1" if a else "0"
    rows[2][12] = "1" if b else "0"
    rows[2] = "".join(rows[2])
    setrow(3, "SLD %04d" % state["sld"])
    setrow(5, "VDD %04dMV" % state["vdd"])
    setrow(6, "MIN%04d MAX%04d" % (state["smin"], state["smax"]))
    setrow(7, state["verdict"])
    return "".join(rows[0]), rows, state

if __name__ == "__main__":
    states = [
        dict(name="healthy board", enc=(12, 1, 0), sld=512, smin=38, smax=1021,
             vdd=2980, verdict="SLIDER OK", btn=[0, 0, 0, 0, 0, 0]),
        dict(name="old board symptom", enc=(-3, 1, 1), sld=1023, smin=1021, smax=1023,
             vdd=3010, verdict="SLIDER RAIL HIGH", btn=[0, 0, 0, 0, 0, 0]),
        dict(name="buttons + encoder", enc=(47, 0, 1), sld=210, smin=12, smax=900,
             vdd=2860, verdict="SLIDER OK", btn=[1, 0, 1, 0, 1, 0]),
    ]
    W, H = 128 * 4, 64 * 4
    out = Image.new("RGB", (W * len(states), H + 16), (30, 30, 30))
    from PIL import ImageDraw
    d = ImageDraw.Draw(out)
    for i, st in enumerate(states):
        _, rows, _ = make(st)
        im = render(rows, st["btn"], st["sld"] >> 2)
        out.paste(im.convert("RGB").resize((W, H), Image.NEAREST), (i * W, 16))
        d.text((i * W + 4, 3), st["name"], fill=(255, 255, 0))
    p = r"D:\GIT\cabnetBoy\software\cabnetBoyTest\_work\selftest_preview.png"
    os.makedirs(os.path.dirname(p), exist_ok=True)
    out.save(p)
    print("wrote", p)
