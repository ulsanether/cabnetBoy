import gerbonara, os, pymupdf
d=r"D:\GIT\cabnetBoy\_work\gerber_fix"
stack = gerbonara.LayerStack.open_dir(d)
print("stack ok")
svg = stack.to_pretty_svg(side='top')
open(r"D:\GIT\cabnetBoy\_work\board_top.svg","w",encoding="utf-8").write(str(svg))
svg = stack.to_pretty_svg(side='bottom')
open(r"D:\GIT\cabnetBoy\_work\board_bottom.svg","w",encoding="utf-8").write(str(svg))
for name in ("board_top","board_bottom"):
    doc=pymupdf.open(rf"D:\GIT\cabnetBoy\_work\{name}.svg")
    pg=doc[0]
    pix=pg.get_pixmap(dpi=300)
    pix.save(rf"D:\GIT\cabnetBoy\_work\{name}.png")
    print(name, pix.width, pix.height)
