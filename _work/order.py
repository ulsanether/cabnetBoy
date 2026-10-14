import os
from gerbonara.rs274x import GerberFile
o=GerberFile.open(r"D:\GIT\cabnetBoy\_work\gerber_fix\Gerber_TopLayer.GTL")
for i,x in enumerate(o.objects[:12]):
    print(i, type(x).__name__, "dark=",getattr(x,'polarity_dark',None))
print("...")
# find index of the big region
for i,x in enumerate(o.objects):
    if type(x).__name__=='Region' and x.polarity_dark:
        bb=x.bounding_box()
        if bb[1][0]-bb[0][0] > 40:
            print("big pour region at index", i, "of", len(o.objects))
            break
print("---- last 6 ----")
for i,x in enumerate(o.objects[-6:]):
    print(type(x).__name__,"dark=",getattr(x,'polarity_dark',None))
