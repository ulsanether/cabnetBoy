import pymupdf
for name in ("board_top","board_bottom"):
    doc=pymupdf.open(rf"D:\GIT\cabnetBoy\_work\{name}.svg")
    pg=doc[0]
    pix=pg.get_pixmap(dpi=1200)
    pix.save(rf"D:\GIT\cabnetBoy\_work\{name}_hi.png")
    print(name, pix.width, pix.height)
