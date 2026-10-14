import os, math, collections, warnings, re
warnings.filterwarnings("ignore")
import shapely
from shapely.geometry import Point, LineString, Polygon
from shapely.ops import unary_union, nearest_points
from shapely.strtree import STRtree
from gerbonara.rs274x import GerberFile

GD = r"D:\GIT\cabnetBoy\_work\gerber_fix"
IN = 25.4

def fix(g):
    if g is None or g.is_empty: return g
    if not g.is_valid:
        try: g = shapely.make_valid(g)
        except Exception: g = g.buffer(0)
    return g

def arc_points(p, n=64):
    cx, cy = p.cx, p.cy
    a1 = math.atan2(p.y1 - cy, p.x1 - cx); a2 = math.atan2(p.y2 - cy, p.x2 - cx)
    r = math.hypot(p.x1 - cx, p.y1 - cy)
    if p.clockwise:
        while a2 > a1 - 1e-12: a2 -= 2*math.pi
    else:
        while a2 < a1 + 1e-12: a2 += 2*math.pi
    return [(cx + r*math.cos(a1 + (a2-a1)*i/n), cy + r*math.sin(a1 + (a2-a1)*i/n)) for i in range(n+1)]

def prim_to_shapely(p):
    n = type(p).__name__
    if n == 'ArcPoly': return fix(Polygon(p.outline))
    if n == 'Line': return fix(LineString([(p.x1,p.y1),(p.x2,p.y2)]).buffer(p.width/2.0, cap_style=1, quad_segs=24))
    if n == 'Circle': return fix(Point(p.x,p.y).buffer(p.r, quad_segs=32))
    if n == 'Rectangle':
        w,h = p.w/2.0, p.h/2.0
        rot = math.radians(getattr(p,'rotation',0.0) or 0.0)
        pts=[(-w,-h),(w,-h),(w,h),(-w,h)]
        pts=[(x*math.cos(rot)-y*math.sin(rot)+p.x, x*math.sin(rot)+y*math.cos(rot)+p.y) for x,y in pts]
        return fix(Polygon(pts))
    if n == 'Arc': return fix(LineString(arc_points(p)).buffer(p.width/2.0, cap_style=2, quad_segs=24))
    try:
        return fix(Polygon(p.to_arc_poly().outline))
    except Exception: return None

def layer_geom(fn):
    o = GerberFile.open(os.path.join(GD, fn))
    result = shapely.GeometryCollection(); pend=[]; pend_dark=None
    def flush():
        nonlocal result, pend, pend_dark
        if not pend: return
        g = unary_union(pend); pend=[]
        result = result.union(g) if pend_dark else result.difference(g)
    for obj in o.objects:
        prims=[prim_to_shapely(x) for x in obj.to_primitives()]
        prims=[x for x in prims if x is not None and not x.is_empty]
        if not prims: continue
        dark=obj.polarity_dark
        if pend_dark is None: pend_dark=dark
        if dark!=pend_dark: flush(); pend_dark=dark
        pend.extend(prims)
    flush(); return o, result

S = lambda g: shapely.affinity.scale(g, IN, IN, origin=(0,0))
print("loading...")
_, gtl = layer_geom("Gerber_TopLayer.GTL")
_, gbl = layer_geom("Gerber_BottomLayer.GBL")
gtl, gbl = S(gtl), S(gbl)

def comps(g):
    return [x for x in (g.geoms if hasattr(g,'geoms') else [g]) if isinstance(x, Polygon) and x.area > 1e-7]

print("\n### ISOLATED PAD CHECK ###")
for nm, fn, lay in (("TOP","Gerber_TopLayer.GTL",gtl), ("BOTTOM","Gerber_BottomLayer.GBL",gbl)):
    o = GerberFile.open(os.path.join(GD, fn))
    pads = []
    for obj in o.objects:
        if type(obj).__name__ == 'Flash':
            gs=[prim_to_shapely(x) for x in obj.to_primitives()]
            gs=[x for x in gs if x]
            if gs:
                pads.append(S(unary_union(gs)))
    cs = comps(lay)
    tree = STRtree(cs)
    islands = []
    for i, p in enumerate(pads):
        idxs = list(tree.query(p, predicate='intersects'))
        tot = sum(cs[j].area for j in idxs)
        # a pad that is its own island has area ~ pad area
        if tot < p.area * 1.15:
            islands.append((round(p.area,3), tuple(round(v,2) for v in p.centroid.coords[0]), tuple(round(v,2) for v in p.bounds)))
    print(nm, "pads:", len(pads), " isolated pads:", len(islands))
    for x in sorted(islands, key=lambda t:-t[0])[:15]:
        print("    pad area", x[0], "mm^2 at", x[1], "bbox", x[2])

print("\n### VIA CHECK (vias connecting nothing) ###")
pth = []
topc = comps(gtl); botc = comps(gbl)
t_tree = STRtree(topc); b_tree = STRtree(botc)
lonely = 0
for dia,x1,y1,x2,y2,ln in pth:
    c = Point(x1*IN, y1*IN)
    ring = c.buffer(dia*IN/2.0*1.6, quad_segs=16)
    ti=[i for i in t_tree.query(ring, predicate='intersects')]
    bi=[i for i in b_tree.query(ring, predicate='intersects')]
    ta = max([topc[i].area for i in ti], default=0)
    ba = max([botc[i].area for i in bi], default=0)
    if ta < 1.0 or ba < 1.0:
        lonely += 1
        print(f"    via at ({x1*IN:.2f},{y1*IN:.2f}) top island area {ta:.2f} bottom {ba:.2f}")
print("vias with a small/unconnected side:", lonely, "of", len(pth))

