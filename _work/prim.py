from gerbonara.rs274x import GerberFile
import shapely
o=GerberFile.open(r"D:\GIT\cabnetBoy\_work\gerber_fix\Gerber_TopLayer.GTL")
x=o.objects[94]
p=list(x.to_primitives())
print("line prims:", len(p), repr(p[0])[:400])
f=[y for y in o.objects if type(y).__name__=='Flash'][0]
pf=list(f.to_primitives())
print("flash prims:", len(pf), repr(pf[0])[:400])
r=o.objects[0]
pr=list(r.to_primitives())
print("region prims:", len(pr), repr(pr[0])[:400])
