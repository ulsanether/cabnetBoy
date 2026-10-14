import os, math, collections, warnings
warnings.filterwarnings("ignore")
import shapely
from shapely.geometry import Point, LineString, Polygon, box, MultiPolygon, GeometryCollection
from shapely.ops import unary_union
from shapely.strtree import STRtree
from gerbonara.rs274x import GerberFile
from gerbonara.excellon import ExcellonFile

GD = r"D:\GIT\cabnetBoy\_work\gerber_fix"
IN = 25.4

def fix(g):
    if g is None or g.is_empty:
        return g
    if not g.is_valid:
        try:
            g = shapely.make_valid(g)
        except Exception:
            g = g.buffer(0)
    return g

def prim_to_shapely(p):
    """gerbonara primitive -> shapely geometry, coords in file units (inch)"""
    n = type(p).__name__
    if n == 'ArcPoly':
        return fix(Polygon(p.outline))
    try:
        ap = p.to_arc_poly()
        return fix(Polygon(ap.outline))
    except Exception:
        return fix(_prim_to_shapely(p))

def _prim_to_shapely(p):
    n = type(p).__name__
    if n == 'Line':
        g = LineString([(p.x1, p.y1), (p.x2, p.y2)])
        return g.buffer(p.width / 2.0, cap_style=1, quad_segs=16)
    if n == 'Rectangle':
        # x,y = center
        w, h = p.w / 2.0, p.h / 2.0
        pts = [(-w, -h), (w, -h), (w, h), (-w, h)]
        rot = getattr(p, 'rotation', 0.0) or 0.0
        a = math.radians(rot)
        pts = [(x * math.cos(a) - y * math.sin(a) + p.x, x * math.sin(a) + y * math.cos(a) + p.y) for x, y in pts]
        return Polygon(pts)
    if n == 'Circle':
        return Point(p.x, p.y).buffer(p.r, quad_segs=24)
    if n == 'ArcPoly':
        return Polygon(p.outline)
    if n == 'Obround':
        return None
    return None

def load_layer(fn):
    f = os.path.join(GD, fn)
    return ExcellonFile.open(f) if fn.endswith('.DRL') else GerberFile.open(f)

def layer_geom(fn):
    o = load_layer(fn)
    result = GeometryCollection()
    pend = []
    pend_dark = None
    def flush():
        nonlocal result, pend, pend_dark
        if not pend:
            return
        g = unary_union(pend)
        pend = []
        result = (result.union(g) if pend_dark else result.difference(g))
    for obj in o.objects:
        prims = [prim_to_shapely(x) for x in obj.to_primitives()]
        prims = [x for x in prims if x is not None and not x.is_empty]
        if not prims:
            continue
        dark = obj.polarity_dark
        if pend_dark is None:
            pend_dark = dark
        if dark != pend_dark:
            flush()
            pend_dark = dark
        pend.extend(prims)
    flush()
    return o, result

def flash_objects(fn):
    o = load_layer(fn)
    out = []
    for obj in o.objects:
        if type(obj).__name__ == 'Flash':
            g = unary_union([prim_to_shapely(x) for x in obj.to_primitives() if prim_to_shapely(x) is not None])
            if not g.is_empty:
                out.append((obj, g))
    return out

print("loading layers ...")
gko_o, gko = layer_geom("Gerber_BoardOutlineLayer.GKO")
gtl_o, gtl = layer_geom("Gerber_TopLayer.GTL")
gbl_o, gbl = layer_geom("Gerber_BottomLayer.GBL")
gto_o, gto = layer_geom("Gerber_TopSilkscreenLayer.GTO")
gbo_o, gbo = layer_geom("Gerber_BottomSilkscreenLayer.GBO")
gts_o, gts = layer_geom("Gerber_TopSolderMaskLayer.GTS")
gbs_o, gbs = layer_geom("Gerber_BottomSolderMaskLayer.GBS")

# ---- board outline: the stroke union has a hole = board area
print("\n=== 1. BOARD OUTLINE ===")
gko_mm = shapely.affinity.scale(gko, IN, IN, origin=(0, 0))
print("outline stroke area mm^2:", round(gko_mm.area, 2), "bbox:", tuple(round(v, 3) for v in gko_mm.bounds))
polys = [g for g in (gko_mm.geoms if hasattr(gko_mm, 'geoms') else [gko_mm]) if isinstance(g, Polygon)]
holes = []
for p in polys:
    for r in p.interiors:
        holes.append(Polygon(r))
print("outline polys:", len(polys), "holes:", len(holes))
board = None
for h in sorted(holes, key=lambda x: -x.area):
    print("  hole area mm^2:", round(h.area, 2), "bounds", tuple(round(v, 2) for v in h.bounds))
    if board is None:
        board = h
if board is not None:
    x0, y0, x1, y1 = board.bounds
    print(f"board size: {x1-x0:.3f} x {y1-y0:.3f} mm  ({(x1-x0)/IN:.4f} x {(y1-y0)/IN:.4f} inch)")

scale = lambda g: shapely.affinity.scale(g, IN, IN, origin=(0, 0))
gtl_mm, gbl_mm = scale(gtl), scale(gbl)
gto_mm, gbo_mm = scale(gto), scale(gbo)
gts_mm, gbs_mm = scale(gts), scale(gbs)
copper = unary_union([gtl_mm, gbl_mm])

print("\n=== 2. COPPER vs BOARD EDGE ===")
if board is not None:
    outside = copper.difference(board)
    print("copper area outside board outline (mm^2):", round(outside.area, 4))
    print("copper to board-edge min distance (mm):", round(copper.distance(board.exterior), 4))
    print("copper total area mm^2:", round(copper.area, 2), " board area mm^2:", round(board.area, 2))

print("\n=== 3. TRACE WIDTHS ===")
for nm, o in (("top", gtl_o), ("bottom", gbl_o)):
    widths = collections.Counter()
    for obj in o.objects:
        if type(obj).__name__ == 'Line':
            widths[round(obj.aperture.diameter * IN, 4)] += 1
    print(nm, sorted(widths.items()))

print("\n=== 4. MIN COPPER CLEARANCE (component-to-component) ===")
def components(g):
    if isinstance(g, Polygon):
        return [g]
    return [x for x in g.geoms if isinstance(x, Polygon) and x.area > 1e-6]
for nm, g in (("top", gtl_mm), ("bottom", gbl_mm)):
    comps = components(g)
    print(nm, "components:", len(comps))
    if len(comps) < 2:
        continue
    tree = STRtree(comps)
    best = []
    for i, c in enumerate(comps):
        for j in tree.query(c, predicate='dwithin', distance=0.35):
            if j <= i:
                continue
            d = c.distance(comps[j])
            best.append((round(d, 4), round(c.area, 3), round(comps[j].area, 3)))
    best.sort()
    print("   smallest gaps (mm, areaA, areaB):", best[:8])

print("\n=== 5. PTH / NPTH DRILLS vs PADS (annular ring) ===")
for fn in ("Drill_PTH_Through.DRL", "Drill_NPTH_Through.DRL"):
    o = ExcellonFile.open(os.path.join(GD, fn))
    cnt = collections.Counter()
    for obj in o.objects:
        cnt[round(obj.aperture.diameter * IN, 3)] += 1
    print(fn, dict(cnt))

pads_top = flash_objects("Gerber_TopLayer.GTL")
pads_bot = flash_objects("Gerber_BottomLayer.GBL")
pth = ExcellonFile.open(os.path.join(GD, "Drill_PTH_Through.DRL"))
flash_pads = pads_top + pads_bot
pad_geoms = [scale(g) for _, g in flash_pads]
tree = STRtree(pad_geoms)
worst = []
for obj in pth.objects:
    d_in = obj.aperture.diameter * IN
    c = Point(obj.x * IN, obj.y * IN)
    hole = c.buffer(d_in / 2.0, quad_segs=32)
    idxs = list(tree.query(hole, predicate='intersects'))
    if len(idxs) == 0:
        worst.append(("NO-PAD", round(d_in, 3), round(obj.x * IN, 3), round(obj.y * IN, 3)))
        continue
    best = None
    for i in idxs:
        pg = pad_geoms[i]
        r = pg.exterior.distance(hole)
        if best is None or r < best:
            best = r
    worst.append((round(best, 4), round(d_in, 3), round(obj.x * IN, 3), round(obj.y * IN, 3)))
worst.sort(key=lambda t: (0 if t[0] == "NO-PAD" else t[0]) if isinstance(t[0], str) else t[0])
print("smallest annular rings (ring_mm, drill_mm, x_mm, y_mm):")
for w in worst[:12]:
    print("   ", w)
print("holes with no pad:", sum(1 for w in worst if w[0] == "NO-PAD"))

print("\n=== 6. SILKSCREEN OVER EXPOSED PADS ===")
for nm, silk, mask, cup in (("top", gto_mm, gts_mm, gtl_mm), ("bottom", gbo_mm, gbs_mm, gbl_mm)):
    exposed = mask.intersection(cup)
    ov = silk.intersection(exposed)
    print(nm, "silk-over-exposed-copper area mm^2:", round(ov.area, 4))
    if ov.area > 0:
        clipped = ov.buffer(0)
        parts = components(clipped)
        parts.sort(key=lambda p: -p.area)
        for p in parts[:6]:
            print("     clash area", round(p.area, 4), "at", tuple(round(v, 2) for v in p.bounds))
