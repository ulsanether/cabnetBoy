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
    return [x for x in (g.geoms if hasattr(g, 'geoms') else [g]) if isinstance(x, Polygon) and x.area > 1e-7]

def pads(fn, lay):
    o = GerberFile.open(os.path.join(GD, fn))
    cs = comps(lay); tree = STRtree(cs); out = []
    for obj in o.objects:
        if type(obj).__name__ != 'Flash':
            continue
        gs = [prim_to_shapely(x) for x in obj.to_primitives()]
        gs = [x for x in gs if x]
        if not gs: continue
        p = S(unary_union(gs))
        idxs = list(tree.query(p, predicate='intersects'))
        tot = sum(cs[j].area for j in idxs)
        out.append((p, cs[idxs[0]] if idxs else None, tot, type(obj).__name__))
    return out, cs

def region_pads(x0, y0, x1, y1):
    print(f"\n--- pads in x[{x0},{x1}] y[{y1},{y0}] ---")
    for fn, lay in (("Gerber_TopLayer.GTL", gtl), ("Gerber_BottomLayer.GBL", gbl)):
        ps, cs = pads(fn, lay)
        for p, c, tot, t in ps:
            cx, cy = p.centroid.x, p.centroid.y
            if x0 <= cx <= x1 and y1 <= cy <= y0:
                bb = p.bounds
                iso = "FLOATING" if (c is None or tot < p.area * 1.15) else f"net {tot:8.2f}"
                print(f"  {fn[6:9]} ({cx:7.2f},{cy:7.2f}) pad_area {p.area:6.3f} bbox {tuple(round(v,2) for v in bb)}  {iso}")

region_pads(-8, -22, 2, -35)
region_pads(-8, -10, -1, -20)
