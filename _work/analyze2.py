import os, math, collections, warnings, re
warnings.filterwarnings("ignore")
import shapely
from shapely.geometry import Point, LineString, Polygon, MultiPolygon, GeometryCollection
from shapely.ops import unary_union
from shapely.strtree import STRtree
from gerbonara.rs274x import GerberFile

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

def arc_points(p, n=64):
    cx, cy = p.cx, p.cy
    a1 = math.atan2(p.y1 - cy, p.x1 - cx)
    a2 = math.atan2(p.y2 - cy, p.x2 - cx)
    r = math.hypot(p.x1 - cx, p.y1 - cy)
    if p.clockwise:
        while a2 > a1 - 1e-12:
            a2 -= 2 * math.pi
    else:
        while a2 < a1 + 1e-12:
            a2 += 2 * math.pi
    return [(cx + r * math.cos(a1 + (a2 - a1) * i / n), cy + r * math.sin(a1 + (a2 - a1) * i / n)) for i in range(n + 1)]

def prim_to_shapely(p):
    n = type(p).__name__
    if n == 'ArcPoly':
        return fix(Polygon(p.outline))
    if n == 'Line':
        return fix(LineString([(p.x1, p.y1), (p.x2, p.y2)]).buffer(p.width / 2.0, cap_style=1, quad_segs=24))
    if n == 'Circle':
        return fix(Point(p.x, p.y).buffer(p.r, quad_segs=32))
    if n == 'Rectangle':
        w, h = p.w / 2.0, p.h / 2.0
        rot = math.radians(getattr(p, 'rotation', 0.0) or 0.0)
        pts = [(-w, -h), (w, -h), (w, h), (-w, h)]
        pts = [(x * math.cos(rot) - y * math.sin(rot) + p.x, x * math.sin(rot) + y * math.cos(rot) + p.y) for x, y in pts]
        return fix(Polygon(pts))
    if n == 'Arc':
        return fix(LineString(arc_points(p)).buffer(p.width / 2.0, cap_style=2, quad_segs=24))
    try:
        ap = p.to_arc_poly()
        return fix(Polygon(ap.outline))
    except Exception:
        return None

def layer_geom(fn):
    o = GerberFile.open(os.path.join(GD, fn))
    result = GeometryCollection()
    pend, pend_dark = [], None
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
            flush(); pend_dark = dark
        pend.extend(prims)
    flush()
    return o, result

def parse_drill(fn):
    """own parser: EasyEDA DRL, INCH, 2 integer 4 decimal, leading zeros omitted"""
    txt = open(os.path.join(GD, fn), encoding="utf-8", errors="replace").read().splitlines()
    holes = []
    diadef = {}
    cur = None
    for line in txt:
        s = line.strip()
        m = re.match(r'^T(\d+)C([\d.]+)$', s)
        if m:
            diadef[m.group(1)] = float(m.group(2)); continue
        m = re.match(r'^T(\d+)$', s)
        if m:
            cur = m.group(1); continue
        m = re.match(r'^X(-?\d+)Y(-?\d+)(?:G85X(-?\d+)Y(-?\d+))?$', s)
        if m:
            def cv(v):
                neg = v.startswith('-')
                v = v.lstrip('-')
                x = int(v) / 10000.0
                return -x if neg else x
            x1, y1 = cv(m.group(1)), cv(m.group(2))
            x2, y2 = (cv(m.group(3)), cv(m.group(4))) if m.group(3) else (x1, y1)
            d = diadef.get(cur)
            holes.append(dict(dia=d, x=x1, y=y1, x2=x2, y2=y2,
                              length=math.hypot(x2 - x1, y2 - y1), tool=cur))
    return [(h['dia'], h['x'], h['y'], h['x2'], h['y2'], h['length']) for h in holes]

print("loading layers ...")
_, gko = layer_geom("Gerber_BoardOutlineLayer.GKO")
_, gtl = layer_geom("Gerber_TopLayer.GTL")
_, gbl = layer_geom("Gerber_BottomLayer.GBL")
_, gto = layer_geom("Gerber_TopSilkscreenLayer.GTO")
_, gbo = layer_geom("Gerber_BottomSilkscreenLayer.GBO")
_, gts = layer_geom("Gerber_TopSolderMaskLayer.GTS")
_, gbs = layer_geom("Gerber_BottomSolderMaskLayer.GBS")
S = lambda g: shapely.affinity.scale(g, IN, IN, origin=(0, 0))
gko, gtl, gbl, gto, gbo, gts, gbs = map(S, (gko, gtl, gbl, gto, gbo, gts, gbs))

print("\n=== 1. BOARD OUTLINE ===")
print("stroke area mm^2:", round(gko.area, 2))
parts = [g for g in (gko.geoms if hasattr(gko, 'geoms') else [gko])]
print("connected pieces:", len(parts), [round(p.area, 2) for p in parts])
holes = []
for p in parts:
    if isinstance(p, Polygon):
        for r in p.interiors:
            holes.append(Polygon(r))
holes.sort(key=lambda h: -h.area)
board = holes[0] if holes else None
if board:
    x0, y0, x1, y1 = board.bounds
    print(f"board: {x1-x0:.3f} x {y1-y0:.3f} mm = {(x1-x0)/IN:.4f} x {(y1-y0)/IN:.4f} inch ; area {board.area:.1f} mm^2")
    print("holes found:", len(holes), [round(h.area, 1) for h in holes[:5]])

copper = unary_union([gtl, gbl])
print("\n=== 2. COPPER vs BOARD OUTLINE ===")
if board:
    out = copper.difference(board)
    print("copper outside board (mm^2):", round(out.area, 4))
    if out.area > 0:
        for p in (out.geoms if hasattr(out, 'geoms') else [out])[:10]:
            print("   outside piece", round(p.area, 4), "at", tuple(round(v, 2) for v in p.bounds))
    print("min copper-to-edge distance (mm):", round(copper.distance(board.exterior), 4))
    from shapely.ops import nearest_points
    p1, p2 = nearest_points(copper, board.exterior)
    print("   nearest points:", tuple(round(v, 2) for v in p1.coords[0]), tuple(round(v, 2) for v in p2.coords[0]))
    print("copper area mm^2:", round(copper.area, 1))

print("\n=== 3. COPPER CLEARANCE (component to component) ===")
def comps(g):
    return [x for x in (g.geoms if hasattr(g, 'geoms') else [g]) if isinstance(x, Polygon) and x.area > 1e-6]
for nm, g in (("top", gtl), ("bottom", gbl)):
    cs = comps(g)
    tree = STRtree(cs)
    res = []
    for i, c in enumerate(cs):
        for j in tree.query(c, predicate='dwithin', distance=0.30):
            if j <= i:
                continue
            d = c.distance(cs[j])
            from shapely.ops import nearest_points
            q1, q2 = nearest_points(c, cs[j])
            res.append((round(d, 4), tuple(round(v, 2) for v in q1.coords[0]), tuple(round(v, 2) for v in q2.coords[0]), round(c.area, 2), round(cs[j].area, 2)))
    res.sort()
    print(nm, "components:", len(cs))
    for r in res[:6]:
        print("   gap", r[0], "mm between", r[1], "and", r[2], "areas", r[3], r[4])

print("\n=== 4. DRILLS ===")
pth = parse_drill("Drill_PTH_Through.DRL")
npth = parse_drill("Drill_NPTH_Through.DRL")
via = parse_drill("Drill_PTH_Through_Via.DRL")
for nm, h in (("PTH", pth), ("NPTH", npth), ("VIA", via)):
    c = collections.Counter(round(x[0] * IN, 3) for x in h)
    print(nm, "count", len(h), "sizes(mm):", dict(sorted(c.items())), " slots:", sum(1 for x in h if x[5] > 1e-6))
print("PTH vs VIA identical sets:", sorted((round(x[0], 5), round(x[1], 5), round(x[2], 5)) for x in pth) ==
      sorted((round(x[0], 5), round(x[1], 5), round(x[2], 5)) for x in via))

print("\n=== 5. ANNULAR RING (PTH) ===")
flashpads = []
for fn in ("Gerber_TopLayer.GTL", "Gerber_BottomLayer.GBL"):
    o = GerberFile.open(os.path.join(GD, fn))
    for obj in o.objects:
        if type(obj).__name__ == 'Flash':
            gs = [prim_to_shapely(x) for x in obj.to_primitives()]
            gs = [x for x in gs if x]
            if gs:
                flashpads.append(S(unary_union(gs)))
padgeoms = flashpads
tree = STRtree(padgeoms)
rows = []
for dia, x1, y1, x2, y2, ln in pth:
    c = Point(x1 * IN, y1 * IN)
    hole = c.buffer(dia * IN / 2.0, quad_segs=32)
    idxs = list(tree.query(hole, predicate='intersects'))
    if not idxs:
        rows.append(("NO-PAD", round(dia * IN, 3), round(x1 * IN, 2), round(y1 * IN, 2), 0))
        continue
    best = min(padgeoms[i].exterior.distance(hole) for i in idxs)
    rows.append((round(best, 4), round(dia * IN, 3), round(x1 * IN, 2), round(y1 * IN, 2), len(idxs)))
rows.sort(key=lambda t: (t[0] if isinstance(t[0], (int, float)) else -1))
print("(ring_mm, drill_mm, x, y, n_pads)")
for r in rows[:10]:
    print("   ", r)
print("holes without any pad:", sum(1 for r in rows if r[0] == "NO-PAD"))
print("min via ring:", min(r[0] for r in rows if isinstance(r[0], float) and abs(r[1] - 0.305) < 0.01))

print("\n=== 6. NPTH holes vs copper ===")
treec = STRtree(comps(copper))
for dia, x1, y1, x2, y2, ln in npth:
    hole = Point(x1 * IN, y1 * IN).buffer(dia * IN / 2.0 + 0.0, quad_segs=32)
    idxs = list(treec.query(hole, predicate='intersects'))
    if idxs:
        d = min(comps(copper)[i].exterior.distance(hole) for i in idxs)
        print(f"   NPTH d={dia*IN:.2f}mm at ({x1*IN:.1f},{y1*IN:.1f}) -> copper at distance {d:.3f} mm")
    else:
        print(f"   NPTH d={dia*IN:.2f}mm at ({x1*IN:.1f},{y1*IN:.1f}) -> no copper within hole")

print("\n=== 7. SILK OVER EXPOSED COPPER (mask opening) ===")
for nm, silk, mask, cup in (("top", gto, gts, gtl), ("bottom", gbo, gbs, gbl)):
    exposed = mask.intersection(cup)
    ov = silk.intersection(exposed)
    print(nm, "overlap area mm^2:", round(ov.area, 4))
    ps = [p for p in (ov.geoms if hasattr(ov, 'geoms') else [ov]) if p.area > 1e-9]
    ps.sort(key=lambda p: -p.area)
    for p in ps[:8]:
        print("     ", round(p.area, 4), "mm^2 at", tuple(round(v, 2) for v in p.bounds))

print("\n=== 8. MASK / PASTE SANITY ===")
mt = gts.intersection(gtl)
mb = gbs.intersection(gbl)
print("top mask openings: n=", len([p for p in (gts.geoms if hasattr(gts, 'geoms') else [gts])]), "area", round(gts.area, 2))
print("bottom mask openings area", round(gbs.area, 2))
print("top mask opening overlapping top copper area", round(mt.area, 2))
print("bottom mask opening overlapping bottom copper area", round(mb.area, 2))
