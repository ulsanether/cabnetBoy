import pymupdf
p=r"D:\GIT\cabnetBoy\Schematic_RetroGame_2026-10-14.pdf"
d=pymupdf.open(p); pg=d[0]
W,H=pg.rect.width,pg.rect.height
regs={
 "S1_u5":(0.60,0.12,0.72,0.26),
 "S2_u6debug":(0.34,0.29,0.47,0.46),
 "S3_rv09":(0.70,0.12,0.84,0.26),
}
for k,(x0,y0,x1,y1) in regs.items():
    pix=pg.get_pixmap(dpi=900, clip=pymupdf.Rect(x0*W,y0*H,x1*W,y1*H))
    f=rf"D:\GIT\cabnetBoy\_work\crops\{k}.png"; pix.save(f); print(k,pix.width,pix.height)
