import pymupdf
d=pymupdf.open(r"D:\GIT\cabnetBoy\CH32V003DS0.PDF")
print("pages",d.page_count)
for i in range(min(6,d.page_count)):
    t=d[i].get_text()
    print("="*20,"P",i+1,"="*20)
    print(t[:1500])
