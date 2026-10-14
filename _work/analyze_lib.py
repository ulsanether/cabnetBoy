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


S = lambda g: shapely.affinity.scale(g, IN, IN, origin=(0, 0))
