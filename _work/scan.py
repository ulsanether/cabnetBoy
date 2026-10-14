import os, pymupdf
from PIL import Image
from gerbonara.rs274x import GerberFile
BOUNDS=((-10.0,-62.5),(42.0,3.5)); DPI=900
o=GerberFile.open(r"D:\GIT\cabnetBoy\_work\gerber_fix\Gerber_TopLayer.GTL")
svg=str(o.to_svg(margin=0.5, force_bounds=BOUNDS, fg='black', bg='white'))
doc=pymupdf.open("svg",svg.encode()); pix=doc[0].get_pixmap(dpi=DPI)
im=Image.frombytes("RGB",(pix.width,pix.height),pix.samples).convert("L")
print("size",im.size)
W,H=im.size
px=im.load()
def runs(vals, start):
    out=[]; cur=None; n=0; s=start
    for i,v in enumerate(vals):
        b = v<128
        if b!=cur:
            if cur is not None: out.append((cur,s,n))
            cur=b; s=start+i; n=1
        else: n+=1
    out.append((cur,s,n))
    return out
for row in (500,549,600):
    vals=[px[x,row] for x in range(1100,1320)]
    print("row",row)
    for dark,s,n in runs(vals,1100):
        print(("  COPPER" if dark else "  gap   "), "x",s,"len",n, "=> mm", round(n/ (DPI/25.4),3))
