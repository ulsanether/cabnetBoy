import pymupdf
d=pymupdf.open(r"D:\GIT\cabnetBoy\Schematic_RetroGame_2026-10-14.pdf")
pg=d[0]
W,H=pg.rect.width,pg.rect.height
words=pg.get_text("words")
# U1 area
x0,y0,x1,y1=0.10*W,0.13*H,0.40*W,0.36*H
sel=[w for w in words if x0<=w[0]<=x1 and y0<=w[1]<=y1]
sel.sort(key=lambda w:(round(w[1],1), w[0]))
for w in sel:
    print(f"x={w[0]:7.1f} y={w[1]:7.1f}  {w[4]}")
