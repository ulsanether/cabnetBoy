import os
from gerbonara.rs274x import GerberFile
o=GerberFile.open(r"D:\GIT\cabnetBoy\_work\gerber_fix\Gerber_TopLayer.GTL")
idx={}
for i,x in enumerate(o.objects):
    t=type(x).__name__
    idx.setdefault(t,[]).append(i)
for t,v in idx.items():
    print(t, "count",len(v), "first",v[0], "last",v[-1])
print("total",len(o.objects))
print("regions first/last:", idx['Region'][0], idx['Region'][-1])
