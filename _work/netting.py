import os, math, warnings, collections
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
    return [x for x in (g.geoms if hasattr(g, 'geoms') else [g]) if isinstance(x, Polygon) and x.area > 1e-7]

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
        idx = idxs[0] if idxs else -1
        out.append(dict(geom=p, comp=idx, layer=tag, c=(round(p.centroid.x, 2), round(p.centroid.y, 2)), area=p.area))
    return out, cs

tp, tc = pads("Gerber_TopLayer.GTL", gtl, "T")
bp, bc = pads("Gerber_BottomLayer.GBL", gbl, "B")

# union-find over components
parent = {}
def find(x):
    parent.setdefault(x, x)
    while parent[x] != x:
        parent[x] = parent[parent[x]]; x = parent[x]
    return x
def union(a, b):
    ra, rb = find(a), find(b)
    if ra != rb: parent[ra] = rb

for i in range(len(tc)): find(("T", i))
for i in range(len(bc)): find(("B", i))

t_tree = STRtree(tc); b_tree = STRtree(bc)
holes = parse_drill("Drill_PTH_Through.DRL") + parse_drill("Drill_NPTH_Through.DRL")
plated = parse_drill("Drill_PTH_Through.DRL")
for dia, x1, y1, x2, y2, ln in plated:
    pt = Point(x1 * IN, y1 * IN).buffer(0.001)
    ti = list(t_tree.query(pt, predicate='intersects'))
    bi = list(b_tree.query(pt, predicate='intersects'))
    if ti and bi:
        union(("T", ti[0]), ("B", bi[0]))

groups = collections.defaultdict(list)
for p in tp: groups[find(("T", p["comp"])) if p["comp"] >= 0 else "ISO_T"].append((p["c"], round(p["area"], 2), "T"))
for p in bp: groups[find(("B", p["comp"])) if p["comp"] >= 0 else "ISO_B"].append((p["c"], round(p["area"], 2), "B"))

# component areas
areas = {}
for i, c in enumerate(tc): areas[find(("T", i))] = areas.get(find(("T", i)), 0) + c.area
for i, c in enumerate(bc): areas[find(("B", i))] = areas.get(find(("B", i)), 0) + c.area

order = sorted(groups.items(), key=lambda kv: -(areas.get(kv[0], 0.0)))
print(f"{len(groups)} copper nets detected")
for k, v in order:
    if k in ("ISO_T", "ISO_B"):
        continue
    pts = sorted(set(x[0] for x in v))
    print(f"\nNET area={areas.get(k,0):8.1f} mm^2  pads={len(v)}")
    print("   ", ", ".join(f"({p[0]:.2f},{p[1]:.2f})" for p in pts))

