"""Compare 'walls bright' vs 'floor bright' terrain rendering."""
import sys, os, importlib
sys.path.insert(0, r"D:\GIT\cabnetBoy\_work")
import rogue_preview as R
from PIL import Image, ImageDraw

PLAYER = [0x3C,0x42,0x42,0x5A,0x5A,0x42,0x42,0x3C]

VAR = {
  "cur wall25/floor dot": dict(floor=[0x00,0x20,0x00,0x02,0x00,0x00,0x88,0x00],
                               wall=[0x88,0x22,0x88,0x22,0x88,0x22,0x88,0x22],
                               void=[0]*8),
  "X floor25 / wall black": dict(floor=[0x88,0x22,0x88,0x22,0x88,0x22,0x88,0x22],
                                 wall=[0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00],
                                 void=[0]*8),
  "Y floor50 / wall black": dict(floor=[0xAA,0x55,0xAA,0x55,0xAA,0x55,0xAA,0x55],
                                 wall=[0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00],
                                 void=[0]*8),
  "Z floor dots / wall50":  dict(floor=[0x00,0x20,0x00,0x02,0x00,0x00,0x88,0x00],
                                 wall=[0xAA,0x55,0xAA,0x55,0xAA,0x55,0xAA,0x55],
                                 void=[0]*8),
  "W floor12 / wall black": dict(floor=[0x80,0x00,0x08,0x00,0x80,0x00,0x08,0x00],
                                 wall=[0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00],
                                 void=[0]*8),
}

def render_with(g, tiles, msg):
    R.TILES = tiles
    return R.render(g, msg)

if __name__ == "__main__":
    seed = 0x7777
    g = R.G(seed); g.depth = 4; g.gen_level(); g.fov()
    names = list(VAR)
    W, H = 128*3, 64*3
    out = Image.new("RGB", (W*len(names), H+14), (30,30,30))
    d = ImageDraw.Draw(out)
    for i, n in enumerate(names):
        v = VAR[n]
        tiles = [v["void"], v["floor"], v["wall"], [0]*8, PLAYER]
        im = render_with(g, tiles, "TERRAIN TEST")
        out.paste(im.convert("RGB").resize((W,H), Image.NEAREST), (i*W, 14))
        d.text((i*W+4, 2), n, fill=(255,255,0))
    p = r"D:\GIT\cabnetBoy\_work\rogue\terrain_compare.png"
    out.save(p)
    print("wrote", p)
