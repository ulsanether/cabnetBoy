import pymupdf, sys, os
p=r"D:\GIT\cabnetBoy\Schematic_RetroGame_2026-10-14.pdf"
d=pymupdf.open(p)
print("pages:", d.page_count)
outdir=r"D:\GIT\cabnetBoy\_work"
os.makedirs(outdir, exist_ok=True)
for i,pg in enumerate(d):
    print("="*30, "PAGE", i+1, "size", pg.rect)
    print(pg.get_text())
    pix = pg.get_pixmap(dpi=150)
    pix.save(os.path.join(outdir, f"schem_p{i+1}.png"))
