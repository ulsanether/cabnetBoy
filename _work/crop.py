import pymupdf, os
p=r"D:\GIT\cabnetBoy\Schematic_RetroGame_2026-10-14.pdf"
d=pymupdf.open(p); pg=d[0]
W,H=pg.rect.width, pg.rect.height
print("page",W,H)
os.makedirs(r"D:\GIT\cabnetBoy\_work\crops", exist_ok=True)
# normalized regions (x0,y0,x1,y1) fractions
regs={
 "A_mcu":(0.045,0.09,0.35,0.50),
 "B_eeprom":(0.04,0.51,0.44,0.87),
 "C_buttons":(0.60,0.30,0.90,0.82),
 "D_power":(0.34,0.09,0.70,0.50),
 "E_audio":(0.43,0.51,0.62,0.87),
}
for k,(x0,y0,x1,y1) in regs.items():
    clip=pymupdf.Rect(x0*W,y0*H,x1*W,y1*H)
    pix=pg.get_pixmap(dpi=400, clip=clip)
    f=rf"D:\GIT\cabnetBoy\_work\crops\{k}.png"
    pix.save(f); print(k, pix.width, pix.height)
