import pymupdf
p=r"D:\GIT\cabnetBoy\Schematic_RetroGame_2026-10-14.pdf"
d=pymupdf.open(p); pg=d[0]
W,H=pg.rect.width,pg.rect.height
regs={
 "R1_u1_right":(0.205,0.145,0.315,0.36),
 "R2_lcd":(0.045,0.52,0.24,0.80),
 "R3_eeprom":(0.17,0.52,0.44,0.80),
 "R4_buzzer":(0.43,0.52,0.60,0.80),
 "R5_power":(0.34,0.10,0.56,0.30),
 "R6_handler":(0.53,0.11,0.83,0.30),
}
for k,(x0,y0,x1,y1) in regs.items():
    pix=pg.get_pixmap(dpi=800, clip=pymupdf.Rect(x0*W,y0*H,x1*W,y1*H))
    f=rf"D:\GIT\cabnetBoy\_work\crops\{k}.png"; pix.save(f); print(k,pix.width,pix.height)
