import os, pymupdf
from PIL import Image
from gerbonara.rs274x import GerberFile
from gerbonara.excellon import ExcellonFile

gd = r"D:\GIT\cabnetBoy\_work\gerber_fix"
layers = {
    "outline": ("Gerber_BoardOutlineLayer.GKO", (255, 0, 255), 0.9),
    "top":     ("Gerber_TopLayer.GTL",          (175, 25, 25), 1.0),
    "bot":     ("Gerber_BottomLayer.GBL",       (25, 25, 175), 1.0),
    "tsilk":   ("Gerber_TopSilkscreenLayer.GTO", (0, 0, 0), 0.9),
    "bsilk":   ("Gerber_BottomSilkscreenLayer.GBO", (0, 0, 0), 0.9),
    "tmask":   ("Gerber_TopSolderMaskLayer.GTS", (0, 180, 60), 0.5),
    "bmask":   ("Gerber_BottomSolderMaskLayer.GBS", (0, 180, 60), 0.5),
    "drill":   ("Drill_PTH_Through.DRL",        (255, 150, 0), 0.9),
    "npth":    ("Drill_NPTH_Through.DRL",       (0, 190, 190), 0.9),
}
BOUNDS = ((-10.0, -62.5), (42.0, 3.5))   # mm, generous margin
DPI = 900

def render(fn, col, alpha_scale):
    f = os.path.join(gd, fn)
    o = ExcellonFile.open(f) if fn.endswith(".DRL") else GerberFile.open(f)
    svg = str(o.to_svg(margin=0.5, force_bounds=BOUNDS, fg='black', bg='white'))
    doc = pymupdf.open("svg", svg.encode())
    pix = doc[0].get_pixmap(dpi=DPI)
    img = Image.frombytes("RGB", (pix.width, pix.height), pix.samples).convert("L")
    a = img.point(lambda v: int((255 - v) * alpha_scale))
    layer = Image.new("RGBA", img.size, col + (0,))
    layer.putalpha(a)
    return layer

imgs = {}
size = None
for k, (fn, col, al) in layers.items():
    imgs[k] = render(fn, col, al)
    size = imgs[k].size
    print(k, size)

canvas = Image.new("RGBA", size, (255, 255, 255, 255))
for k in ["outline", "top", "bot", "tsilk", "bsilk", "tmask", "bmask", "drill", "npth"]:
    canvas = Image.alpha_composite(canvas, imgs[k])
canvas.convert("RGB").save(r"D:\GIT\cabnetBoy\_work\board_all.png")
print("saved", canvas.size)

for view, keys in (("top", ["outline", "top", "tsilk", "tmask", "drill", "npth"]),
                   ("bottom", ["outline", "bot", "bsilk", "bmask", "drill", "npth"])):
    c = Image.new("RGBA", size, (255, 255, 255, 255))
    for k in keys:
        c = Image.alpha_composite(c, imgs[k])
    c.convert("RGB").save(rf"D:\GIT\cabnetBoy\_work\board_{view}.png")
    print("saved", view)

