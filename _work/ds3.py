import pymupdf
d=pymupdf.open(r"D:\GIT\cabnetBoy\CH32V003DS0.PDF")
t=d[11].get_text()
i=t.find("CH32V003A4M6")
print(t[i:i+1800])
print("#### PAGE 13 ####")
print(d[12].get_text()[:4000])
