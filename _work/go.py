import gerbonara.graphic_objects as go
print([n for n in dir(go) if n[0].isupper()])
print()
print([m for m in dir(go.Line) if not m.startswith('__')])
print()
print([m for m in dir(go.ArcPoly) if not m.startswith('__')])
import inspect
print(inspect.signature(go.Line.to_shapely) if hasattr(go.Line,'to_shapely') else 'no Line.to_shapely')
