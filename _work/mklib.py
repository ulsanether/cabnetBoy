import re
src=open(r"D:\GIT\cabnetBoy\_work\analyze2.py",encoding="utf-8").read()
cut=src.index('print("loading layers ...")')
open(r"D:\GIT\cabnetBoy\_work\analyze_lib.py","w",encoding="utf-8").write(src[:cut])
print("lib written")
