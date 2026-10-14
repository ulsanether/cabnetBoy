import pymupdf
p=r"D:\GIT\cabnetBoy\Schematic_RetroGame_2026-10-14.pdf"
d=pymupdf.open(p); pg=d[0]
W,H=pg.rect.width, pg.rect.height
clip=pymupdf.Rect(0.14*W,0.11*H,0.32*W,0.42*H)
pix=pg.get_pixmap(dpi=700, clip=clip); pix.save(r"D:\GIT\cabnetBoy\_work\crops\A2_mcu_zoom.png"); print(pix.width,pix.height)
clip=pymupdf.Rect(0.20*W,0.11*H,0.34*W,0.42*H)
pix=pg.get_pixmap(dpi=700, clip=clip); pix.save(r"D:\GIT\cabnetBoy\_work\crops\A3_mcu_right.png"); print(pix.width,pix.height)
