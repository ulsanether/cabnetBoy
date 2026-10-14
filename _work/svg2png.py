import pymupdf, os, glob
svgd=r"D:\GIT\cabnetBoy\_work\svg"
outd=r"D:\GIT\cabnetBoy\_work\png"
os.makedirs(outd, exist_ok=True)
for f in sorted(glob.glob(os.path.join(svgd,"*.svg"))):
    base=os.path.basename(f)[:-4]
    try:
        doc=pymupdf.open(f)
        pg=doc[0]
        pix=pg.get_pixmap(dpi=200)
        pix.save(os.path.join(outd, base+".png"))
        print(base, pix.width, pix.height)
    except Exception as e:
        print(base,"ERR",type(e).__name__,e)
