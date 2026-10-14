import os, collections
from gerbonara.rs274x import GerberFile
gd=r"D:\GIT\cabnetBoy\_work\gerber_fix"
o=GerberFile.open(os.path.join(gd,"Gerber_TopLayer.GTL"))
types=collections.Counter()
for x in o.objects:
    types[(type(x).__name__, getattr(x,'polarity_dark',None))]+=1
print(dict(types))
lines=[x for x in o.objects if type(x).__name__=='Line']
print("first 6 lines:")
for x in lines[:6]:
    print("  ", x, "pol=",x.polarity_dark, "ap=",getattr(x,'aperture',None))
regs=[x for x in o.objects if type(x).__name__=='Region']
print("first 4 regions:")
for x in regs[:4]:
    bb=x.bounding_box()
    print("   pol=",x.polarity_dark, "bbox=",bb)
# large dark regions
big=[x for x in regs if x.polarity_dark]
big.sort(key=lambda r: -( (r.bounding_box()[1][0]-r.bounding_box()[0][0])*(r.bounding_box()[1][1]-r.bounding_box()[0][1]) ))
for x in big[:3]:
    print("BIG dark region bbox", x.bounding_box())
