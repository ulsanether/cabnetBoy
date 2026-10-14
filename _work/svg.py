import gerbonara, os, glob
d=r"D:\GIT\cabnetBoy\_work\gerber"
out=r"D:\GIT\cabnetBoy\_work\svg"
os.makedirs(out, exist_ok=True)
for f in sorted(glob.glob(os.path.join(d,"*.G*"))):
    base=os.path.basename(f).split(".")[0]
    try:
        g=gerbonara.rs274x.GerberFile.open(f)
        print(base, "objects:", len(g.objects), "bounds:", g.bounding_box())
        svg=g.to_svg(margin=1.0)
        open(os.path.join(out, base+".svg"),"w",encoding="utf-8").write(str(svg))
        print("   svg chars", len(str(svg)))
    except Exception as e:
        print(base, "ERR", type(e).__name__, e)
