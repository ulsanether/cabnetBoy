"""Feature test screen: room + corridor + monsters + items + stairs."""
import sys, os
sys.path.insert(0, r"D:\GIT\cabnetBoy\_work")
import rogue_preview as R
from PIL import Image

g = R.G(1)
g.map = [R.TER_WALL] * (R.MAP_W * R.MAP_H)
def room(x0, y0, x1, y1, t=R.TER_FLOOR):
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            g.mset(x, y, t)
room(2, 2, 9, 7)          # start room
room(12, 9, 19, 14)       # far room
for x in range(7, 15):    # corridor
    g.mset(x, 4, R.TER_FLOOR)
for y in range(4, 12):
    g.mset(14, y, R.TER_FLOOR)
g.mset(14, 4, R.TER_DOOR)
g.mset(19, 14, R.TER_STAIR)
g.px, g.py, g.facing = 5, 5, 1
g.mons = [[7, 4, 4, 0], [8, 6, 6, 1], [13, 9, 5, 2]]
g.items = [[3, 3, 0], [6, 6, 1]]
g.hp, g.hp_max, g.depth, g.gold = 9, 12, 3, 7
g.fov()
img = R.render(g, "IT HITS YOU")
p = r"D:\GIT\cabnetBoy\_work\rogue\feature_test.png"
img.resize((128 * 5, 64 * 5), Image.NEAREST).save(p)
print("wrote", p)
print("visible tiles:", sum(g.vis), "seen:", sum(g.seen))
