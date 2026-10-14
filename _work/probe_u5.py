"""What is U5 (the B10K slider) actually wired to on the PCB?

Rebuilds the copper netlist from the Gerber files and prints every pad in the
U5 / RV09 region together with the size of the copper net it belongs to.

Reference sizes:
  GND plane  -> ~2580 mm^2     VCC -> ~90 mm^2
  a 2-pad signal net -> ~2-3 mm^2      pad that touches nothing -> equals its own area
"""
import os, warnings, collections
warnings.filterwarnings("ignore")
import shapely
from shapely.geometry import Point, Polygon
from shapely.ops import unary_union
from shapely.strtree import STRtree
from gerbonara.rs274x import GerberFile
from analyze_lib import layer_geom, prim_to_shapely, S, GD, IN, parse_drill

_, gtl = layer_geom("Gerber_TopLayer.GTL")
_, gbl = layer_geom("Gerber_BottomLayer.GBL")
gtl, gbl = S(gtl), S(gbl)

def comps(g):
    return [x for x in (g.geoms if hasattr(g, 'geoms') else [g])
            if isinstance(x, Polygon) and x.area > 1e-7]

def pads(fn, lay, tag):
    o = GerberFile.open(os.path.join(GD, fn))
    cs = comps(lay)
    tree = STRtree(cs)
    out = []
    for obj in o.objects:
        if type(obj).__name__ != 'Flash':
            continue
        gs = [prim_to_shapely(x) for x in obj.to_primitives()]
        gs = [x for x in gs if x]
        if not gs:
            continue
        p = S(unary_union(gs))
        idxs = list(tree.query(p, predicate='intersects'))
        out.append(dict(g=p, comp=idxs[0] if idxs else -1, tag=tag,
                        c=(round(p.centroid.x, 2), round(p.centroid.y, 2)),
                        area=p.area, lay=lay, cs=cs))
    return out

tp = pads("Gerber_TopLayer.GTL", gtl, "T")
bp = pads("Gerber_BottomLayer.GBL", gbl, "B")

# union-find so plated holes merge top and bottom copper
parent = {}
def find(x):
    parent.setdefault(x, x)
    while parent[x] != x:
        parent[x] = parent[parent[x]]
        x = parent[x]
    return x
def union(a, b):
    ra, rb = find(a), find(b)
    if ra != rb:
        parent[ra] = rb

tc, bc = comps(gtl), comps(gbl)
for i in range(len(tc)): find(("T", i))
for i in range(len(bc)): find(("B", i))
t_tree, b_tree = STRtree(tc), STRtree(bc)
for dia, x1, y1, x2, y2, ln in parse_drill("Drill_PTH_Through.DRL"):
    pt = Point(x1 * IN, y1 * IN).buffer(0.001)
    ti = list(t_tree.query(pt, predicate='intersects'))
    bi = list(b_tree.query(pt, predicate='intersects'))
    if ti and bi:
        union(("T", ti[0]), ("B", bi[0]))

areas = {}
for i, c in enumerate(tc):
    k = find(("T", i)); areas[k] = areas.get(k, 0) + c.area
for i, c in enumerate(bc):
    k = find(("B", i)); areas[k] = areas.get(k, 0) + c.area

def netarea(pad):
    if pad['comp'] < 0:
        return 0.0
    return areas.get(find(("T" if pad['tag'] == "T" else "B", pad['comp'])), 0.0)

def dump(x0, x1, y0, y1, title):
    print(f"\n===== {title}  x[{x0},{x1}] y[{y1},{y0}] =====")
    rows = []
    for p in tp + bp:
        cx, cy = p['c']
        if not (x0 <= cx <= x1 and y1 <= cy <= y0):
            continue
        na = netarea(p)
        if na > 1000:
            kind = "GND"
        elif na > 50:
            kind = "VCC?"
        elif na < p['area'] * 1.15:
            kind = "FLOATING"
        else:
            kind = "signal"
        rows.append((cy, cx, p['tag'], round(p['area'], 2), round(na, 2), kind))
    rows.sort(key=lambda r: (r[1], -r[0]))
    print(f"{'x':>7} {'y':>7} {'lay':>3} {'pad':>6} {'net':>9}  kind")
    for cy, cx, tag, pa, na, kind in rows:
        print(f"{cx:7.2f} {cy:7.2f} {tag:>3} {pa:6.2f} {na:9.2f}  {kind}")

dump(29, 42, -4, -32, "U5 B10K slider region")
dump(17, 27, -39, -52, "RV09 HANDLER region")

