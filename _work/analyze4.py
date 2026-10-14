import os, math, warnings, collections
warnings.filterwarnings("ignore")
import shapely
from shapely.geometry import Point, LineString, Polygon
from shapely.ops import unary_union
from shapely.strtree import STRtree
from gerbonara.rs274x import GerberFile
from analyze3 import layer_geom, prim_to_shapely, S, GD, IN

_, gtl = layer_geom("Gerber_TopLayer.GTL")
_, gbl = layer_geom("Gerber_BottomLayer.GBL")
gtl, gbl = S(gtl), S(gbl)

def comps(g):
    return [x for x in (g.geoms if hasattr(g, 'geoms') else [g]) if isinstance(x, Polygon) and x.area > 1e-7]

def islands(fn, lay):
    o = GerberFile.open(os.path.join(GD, fn))
    cs = comps(lay)
    tree = STRtree(cs)
    out = {}
    for obj in o.objects:
        if type(obj).__name__ != 'Flash':
            continue
        gs = [prim_to_shapely(x) for x in obj.to_primitives()]
        gs = [x for x in gs if x]
        if not gs:
            continue
        p = S(unary_union(gs))
        idxs = list(tree.query(p, predicate='intersects'))
        tot = sum(cs[j].area for j in idxs)
        cx, cy = p.centroid.coords[0]
        out[(round(cx, 2), round(cy, 2))] = (round(p.area, 3), tot, round(cx, 2), round(cy, 2))
    return out

ti = islands("Gerber_TopLayer.GTL", gtl)
bi = islands("Gerber_BottomLayer.GBL", gbl)
print("top pads", len(ti), "bottom pads", len(bi))
print("\n# pads isolated on BOTH layers (floating copper):")
for k in sorted(set(ti) & set(bi), key=lambda k: ti[k][2:]):
    a1, t1, x, y = ti[k]
    a2, t2, _, _ = bi[k]
    if t1 < a1 * 1.15 and t2 < a2 * 1.15:
        print(f"   ({x:7.2f},{y:7.2f})  top area {a1:7.3f} (net {t1:8.3f})  bottom area {a2:7.3f} (net {t2:8.3f})")
print("\n# pads isolated on TOP only:")
for k in sorted(set(ti) - set(bi)):
    a1, t1, x, y = ti[k]
    if t1 < a1 * 1.15:
        print(f"   ({x:7.2f},{y:7.2f}) area {a1:.3f}")
print("\n# pads isolated on BOTTOM only:")
for k in sorted(set(bi) - set(ti)):
    a2, t2, x, y = bi[k]
    if t2 < a2 * 1.15:
        print(f"   ({x:7.2f},{y:7.2f}) area {a2:.3f}")
