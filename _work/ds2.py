import pymupdf, re
d=pymupdf.open(r"D:\GIT\cabnetBoy\CH32V003DS0.PDF")
for i in range(d.page_count):
    t=d[i].get_text()
    if "Pin" in t and ("PC1" in t or "Pin definition" in t.lower() or "Pin assignment" in t.lower()):
        print("="*25,"PAGE",i+1,"="*25)
        print(t[:3000])
