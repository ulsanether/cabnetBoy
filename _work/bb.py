import os
from gerbonara.rs274x import GerberFile
from gerbonara.excellon import ExcellonFile
gd=r"D:\GIT\cabnetBoy\_work\gerber_fix"
for fn in ["Gerber_TopLayer.GTL","Drill_PTH_Through.DRL","Drill_NPTH_Through.DRL","Gerber_TopSolderMaskLayer.GTS","Gerber_BoardOutlineLayer.GKO"]:
    f=os.path.join(gd,fn)
    o = ExcellonFile.open(f) if fn.endswith(".DRL") else GerberFile.open(f)
    print(fn, "|", repr(o.bounding_box()))
