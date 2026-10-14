"""Host-side preview of the cabnetBoyRogue screen.

Mirrors src/gfx.c + the drawing half of src/game.c so the font, the terrain
tiles and the level generator can be checked without flashing the MCU.
Font and tile bitmaps are parsed straight out of the C sources, so anything
that looks wrong here is wrong in the firmware too.
"""
import re, os, sys
from PIL import Image

SRC = r"D:\GIT\cabnetBoy\software\cabnetBoyRogue\src"

# ---------------------------------------------------------------- parse ----
def parse_font():
    txt = open(os.path.join(SRC, "font8.h"), encoding="utf-8").read()
    body = txt.split("= {", 1)[1].split("};", 1)[0]
    body = re.sub(r"//[^\n]*", "", body)
    vals = [int(v, 16) for v in re.findall(r"0x([0-9A-Fa-f]{2})", body)]
    return [vals[i * 8:(i + 1) * 8] for i in range(len(vals) // 8)]

def parse_tiles():
    txt = open(os.path.join(SRC, "tiles.h"), encoding="utf-8").read()
    body = txt.split("= {", 1)[1].split("};", 1)[0]
    body = re.sub(r"//[^\n]*", "", body)
    vals = [int(v, 16) for v in re.findall(r"0x([0-9A-Fa-f]{2})", body)]
    return [vals[i * 8:(i + 1) * 8] for i in range(len(vals) // 8)]

FONT8 = parse_font()
TILES = parse_tiles()

MAP_W, MAP_H, FOV_R = 32, 16, 7
VIEW_W, VIEW_H = 16, 6
T_VOID, T_FLOOR, T_WALL, T_RUBBLE = 0, 1, 2, 3
TER_WALL, TER_FLOOR, TER_DOOR, TER_STAIR = 0, 1, 2, 3
MON_CH = "RGSO"

# ------------------------------------------------------------ game state ---
class G:
    def __init__(self, seed):
        self.rng = seed & 0xFFFF or 0xACE1
        self.map = [0] * (MAP_W * MAP_H)
        self.seen = [0] * (MAP_W * MAP_H)
        self.vis = [0] * (MAP_W * MAP_H)
        self.mons = []
        self.items = []
        self.px = self.py = 0
        self.facing = 1
        self.depth = 1
        self.gold = 0
        self.hp = self.hp_max = 12
        self.mp = 8
        self.power = 3
        self.weapon = "SWD"

    def rnd(self):
        s = self.rng
        s ^= (s << 7) & 0xFFFF
        s ^= s >> 9
        s ^= (s << 8) & 0xFFFF
        self.rng = s & 0xFFFF
        return self.rng

    def rnd_n(self, n):
        return self.rnd() % n if n else 0

    def mget(self, x, y): return self.map[y * MAP_W + x]
    def mset(self, x, y, v): self.map[y * MAP_W + x] = v
    def bget(self, bm, x, y): return bm[y * MAP_W + x]
    def bset(self, bm, x, y): bm[y * MAP_W + x] = 1
    def walkable(self, x, y): return self.mget(x, y) in (TER_FLOOR, TER_DOOR, TER_STAIR)

    def mon_at(self, x, y):
        for i, m in enumerate(self.mons):
            if m[2] and m[0] == x and m[1] == y:
                return i
        return None

    def item_at(self, x, y):
        for i, it in enumerate(self.items):
            if it[2] != 0xFF and it[0] == x and it[1] == y:
                return i
        return None

    # ------------------------------------------------------ level gen -----
    def gen_level(self):
        self.map = [TER_WALL] * (MAP_W * MAP_H)
        self.seen = [0] * (MAP_W * MAP_H)
        self.vis = [0] * (MAP_W * MAP_H)
        self.mons, self.items = [], []
        nrooms = 4
        rooms = []
        for i in range(nrooms):
            tries = 0
            while True:
                w = 4 + self.rnd_n(4)
                h = 2 + self.rnd_n(3)
                x = 1 + self.rnd_n(MAP_W - w - 2)
                y = 1 + self.rnd_n(MAP_H - h - 2)
                r = (x, y, w, h)
                if self.room_free(r):
                    break
                tries += 1
                if tries > 30:
                    r = (2 + i * 6, 2 + (i & 1) * 5, 4, 2)
                    break
            rooms.append(r)
            for yy in range(r[1], r[1] + r[3]):
                for xx in range(r[0], r[0] + r[2]):
                    self.mset(xx, yy, TER_FLOOR)

        for i in range(1, nrooms):
            cx0, cy0 = rooms[i - 1][0] + rooms[i - 1][2] // 2, rooms[i - 1][1] + rooms[i - 1][3] // 2
            cx1, cy1 = rooms[i][0] + rooms[i][2] // 2, rooms[i][1] + rooms[i][3] // 2
            for x in range(min(cx0, cx1), max(cx0, cx1) + 1):
                self.mset(x, cy0, TER_FLOOR)
            for y in range(min(cy0, cy1), max(cy0, cy1) + 1):
                self.mset(cx1, y, TER_FLOOR)

        self.px = rooms[0][0] + rooms[0][2] // 2
        self.py = rooms[0][1] + rooms[0][3] // 2
        self.facing = 1
        self.mset(rooms[-1][0] + rooms[-1][2] // 2, rooms[-1][1] + rooms[-1][3] // 2, TER_STAIR)

        for i in range(1, nrooms):
            cnt = 1 + self.rnd_n(2 + self.depth // 3)
            while cnt > 0 and len(self.mons) < 10:
                cnt -= 1
                x = rooms[i][0] + self.rnd_n(rooms[i][2])
                y = rooms[i][1] + self.rnd_n(rooms[i][3])
                if not self.walkable(x, y) or (x == self.px and y == self.py):
                    continue
                if self.mon_at(x, y) is not None:
                    continue
                kind = self.rnd_n(2) + (1 if self.depth > 2 else 0) + (1 if self.depth > 5 else 0)
                kind = min(kind, 3)
                self.mons.append([x, y, 3 + kind * 2, kind])

        for i in range(3):
            if len(self.items) >= 6:
                break
            r = 1 + self.rnd_n(3)
            x = rooms[r][0] + self.rnd_n(rooms[r][2])
            y = rooms[r][1] + self.rnd_n(rooms[r][3])
            if not self.walkable(x, y) or self.item_at(x, y) is not None:
                continue
            self.items.append([x, y, 1 if self.rnd_n(3) == 0 else 0])

    def room_free(self, r):
        for y in range(r[1] - 1, r[1] + r[3] + 1):
            for x in range(r[0] - 1, r[0] + r[2] + 1):
                if x < 0 or y < 0 or x >= MAP_W or y >= MAP_H:
                    continue
                if self.mget(x, y) != TER_WALL:
                    return False
        return True

    # ------------------------------------------------------------- los ----
    def los(self, x0, y0, x1, y1):
        ax, ay, bx, by = x0, y0, x1, y1
        dx, dy = abs(bx - ax), -abs(by - ay)
        sx = 1 if ax < bx else -1
        sy = 1 if ay < by else -1
        err = dx + dy
        while True:
            if ax == bx and ay == by:
                return True
            e2 = err * 2
            if e2 >= dy:
                err += dy
                ax += sx
            if e2 <= dx:
                err += dx
                ay += sy
            if ax == bx and ay == by:
                return True
            if not (0 <= ax < MAP_W and 0 <= ay < MAP_H):
                return False
            if self.mget(ax, ay) == TER_WALL:
                return False

    def fov(self):
        self.vis = [0] * (MAP_W * MAP_H)
        self.bset(self.vis, self.px, self.py)
        self.bset(self.seen, self.px, self.py)
        for dy in range(-FOV_R, FOV_R + 1):
            for dx in range(-FOV_R, FOV_R + 1):
                if dx * dx + dy * dy > FOV_R * FOV_R:
                    continue
                x, y = self.px + dx, self.py + dy
                if not (0 <= x < MAP_W and 0 <= y < MAP_H):
                    continue
                if self.los(self.px, self.py, x, y):
                    self.bset(self.vis, x, y)
                    self.bset(self.seen, x, y)

    # ------------------------------------------------------- drawing ------
    def cell_code(self, x, y):
        if x == self.px and y == self.py:
            return 4 + self.facing      # G_PL_N + facing
        if not self.bget(self.seen, x, y):
            return T_VOID
        if self.bget(self.vis, x, y):
            mi = self.mon_at(x, y)
            if mi is not None:
                return ord(MON_CH[self.mons[mi][3]])
            ii = self.item_at(x, y)
            if ii is not None:
                return ord('$') if self.items[ii][2] == 1 else ord('!')
        t = self.mget(x, y)
        if t == TER_WALL:
            return T_WALL
        if t == TER_DOOR:
            return ord('+')
        if t == TER_STAIR:
            return ord('>')
        return T_FLOOR

    def status(self):
        return "HP%02d M%02d P%d %s" % (self.hp, self.mp, self.power, self.weapon)

# -------------------------------------------------------------- render -----
def glyph_of(value):
    if value < 32:
        if value >= len(TILES):
            value = T_VOID
        return TILES[value]
    if value > 95:
        value = 32
    return FONT8[value - 32]

def render(g, msg):
    img = Image.new("1", (128, 64), 0)
    px = img.load()
    for page in range(8):
        if page < VIEW_H:
            cam_x = max(0, min(g.px - VIEW_W // 2, MAP_W - VIEW_W))
            cam_y = max(0, min(g.py - VIEW_H // 2, MAP_H - VIEW_H))
            cells = [g.cell_code(cam_x + c, cam_y + page) for c in range(VIEW_W)]
        else:
            text = g.status() if page == 6 else msg
            cells = [ord(ch.upper()) for ch in (text + " " * 16)[:16]]
        for col, val in enumerate(cells):
            bmp = glyph_of(val)
            for x in range(8):
                for y in range(8):
                    if bmp[y] & (0x80 >> x):
                        px[col * 8 + x, page * 8 + y] = 1
    return img

if __name__ == "__main__":
    outdir = r"D:\GIT\cabnetBoy\_work\rogue"
    os.makedirs(outdir, exist_ok=True)
    for seed in (0xACE1, 0x1234, 0x7777):
        g = G(seed)
        g.depth = int(sys.argv[1]) if len(sys.argv) > 1 else 1
        g.gen_level()
        g.fov()
        img = render(g, "CABNETBOY ROGUE")
        img = img.resize((128 * 5, 64 * 5), Image.NEAREST)
        img.save(os.path.join(outdir, f"screen_{seed:04X}.png"))
        print(f"seed {seed:04X}: rooms ok, monsters={len(g.mons)}, items={len(g.items)}, "
              f"player=({g.px},{g.py})")
    print("wrote", outdir)

