import os, shapely
from shapely.geometry import Polygon, GeometryCollection
from shapely.ops import unary_union
from gerbonara.rs274x import GerberFile
GD=r"D:\GIT\cabnetBoy\_work\gerber_fix"
o=GerberFile.open(os.path.join(GD,"Gerber_BoardOutlineLayer.GKO"))
print("objects:",len(o.objects))
geoms=[]
for i,obj in enumerate(o.objects):
    ps=[]
    for p in obj.to_primitives():
        ap=p.to_arc_poly()
        g=Polygon(ap.outline)
        if not g.is_valid:
            try: g=shapely.make_valid(g)
            except Exception: g=g.buffer(0)
        ps.append(g)
    g=unary_union(ps)
    geoms.append(g)
    print(i, type(obj).__name__, [type(p).__name__ for p in obj.to_primitives()], tuple(round(v,3) for v in g.bounds))
u=unary_union(geoms)
print("union type", u.geom_type, "area", round(u.area,4))
parts = list(u.geoms) if hasattr(u,'geoms') else [u]
for p in parts:
    print("  part", p.geom_type, round(p.area,4), tuple(round(v,3) for v in p.bounds), "holes", len(getattr(p,'interiors',[])))
