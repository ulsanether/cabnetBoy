import gerbonara, inspect
from gerbonara.rs274x import GerberFile
print(inspect.signature(GerberFile.to_svg))
print(inspect.signature(GerberFile.to_pretty_svg))
g=GerberFile.open(r"D:\GIT\cabnetBoy\_work\gerber\Gerber_TopLayer.GTL")
s=str(g.to_svg(margin=1))
print(s[:1200])
