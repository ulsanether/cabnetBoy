import os, collections
from gerbonara.rs274x import GerberFile
GD=r"D:\GIT\cabnetBoy\_work\gerber_fix"
names=collections.Counter()
attrs={}
for fn in ["Gerber_TopLayer.GTL","Gerber_BottomLayer.GBL","Gerber_BoardOutlineLayer.GKO","Gerber_TopSilkscreenLayer.GTO","Gerber_BottomSilkscreenLayer.GBO","Gerber_TopSolderMaskLayer.GTS","Gerber_BottomSolderMaskLayer.GBS","Gerber_DocumentLayer.GDL"]:
    o=GerberFile.open(os.path.join(GD,fn))
    for obj in o.objects:
        for p in obj.to_primitives():
            n=type(p).__name__
            names[n]+=1
            if n not in attrs:
                attrs[n]=[a for a in dir(p) if not a.startswith('_')]
print(dict(names))
for k,v in attrs.items(): print(k, v)
