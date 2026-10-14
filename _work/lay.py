import os, pymupdf, collections
from PIL import Image
from gerbonara.rs274x import GerberFile
from gerbonara.excellon import ExcellonFile
gd=r"D:\GIT\cabnetBoy\_work\gerber_fix"
BOUNDS=((-10.0,-62.5),(42.0,3.5))
for fn in ["Gerber_TopLayer.GTL","Gerber_BottomLayer.GBL"]:
    o=GerberFile.open(os.path.join(gd,fn))
    c=collections.Counter(type(x).__name__ for x in o.objects)
    pol=collections.Counter(getattr(x,'polarity_dark',None) for x in o.objects)
    print(fn, dict(c))
    print("   polarity:", dict(pol))
    # region sizes
    regs=[x for x in o.objects if type(x).__name__=='Region']
    print("   n regions:", len(regs))
