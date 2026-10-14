from gerbonara.excellon import ExcellonFile
f=r"D:\GIT\cabnetBoy\_work\gerber_fix\Drill_PTH_Through.DRL"
o=ExcellonFile.open(f)
print("n objects", len(o.objects))
for x in o.objects[:6]: print(x, getattr(x,'x',None), getattr(x,'y',None), getattr(x,'diameter',None))
print("bbox", o.bounding_box())
print("unit", o.unit)
