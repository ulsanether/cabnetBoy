try:
    import shapely; print("shapely", shapely.__version__)
except Exception as e: print("no shapely", e)
import gerbonara
from gerbonara.graphic_objects import Line, Flash, Region
import inspect
print([m for m in dir(Line) if 'prim' in m.lower() or 'shapely' in m.lower() or 'poly' in m.lower()])
print([m for m in dir(Flash) if 'prim' in m.lower() or 'shapely' in m.lower() or 'poly' in m.lower()])
print([m for m in dir(Region) if 'prim' in m.lower() or 'shapely' in m.lower() or 'poly' in m.lower()])
print(inspect.signature(Line.to_primitives) if hasattr(Line,'to_primitives') else 'no to_primitives')
