"""Four-facing + power test sheet.

Mirrors the render half of the new game.c. Verifies the directional @ tiles,
the HUD line (HP / MP / power / weapon) and a spell bolt scene.
"""
import sys, os
sys.path.insert(0, r"D:\GIT\cabnetBoy\_work")
import rogue_preview as R
from PIL import Image, ImageDraw

def scene(g, msg):
    return R.render(g, msg)

g = R.G(0xACE1)
g.map = [R.TER_WALL] * (R.MAP_W * R.MAP_H)
def room(x0, y0, x1, y1, t=R.TER_FLOOR):
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            g.mset(x, y, t)
room(2, 2, 11, 7)
room(14, 8, 21, 14)
for x in range(8, 16):
    g.mset(x, 4, R.TER_FLOOR)
for y in range(4, 10):
    g.mset(15, y, R.TER_FLOOR)
g.mset(15, 4, R.TER_DOOR)
g.mset(21, 14, R.TER_STAIR)
g.px, g.py = 5, 4
g.mons = [[9, 4, 4, 0], [8, 6, 6, 1], [16, 9, 5, 2]]
g.items = [[3, 3, 0], [6, 6, 1]]
g.hp, g.mp, g.depth, g.gold = 9, 6, 3, 7

W, H = 128 * 4, 64 * 4
out = Image.new("RGB", (W * 4, H + 16), (30, 30, 30))
d = ImageDraw.Draw(out)
labels = ["facing N  P3", "facing E  P3", "facing S  P1 SWD", "facing W  P5 STF"]
for i, (f, p, w) in enumerate([(0, 3, "SWD"), (1, 3, "SWD"), (2, 1, "DAG"), (3, 5, "STF")]):
    g.facing, g.power, g.weapon = f, p, w
    g.fov()
    msg = ["TURN THE KNOB", "STEP FORWARD", "POWER 1 DAGGER", "POWER 5 MAGIC"]
    im = scene(g, msg[i])
    out.paste(im.convert("RGB").resize((W, H), Image.NEAREST), (i * W, 16))
    d.text((i * W + 4, 3), labels[i], fill=(255, 255, 0))
p = r"D:\GIT\cabnetBoy\_work\rogue\facing_test.png"
out.save(p)
print("wrote", p)
