import json
p=r"D:\GIT\cabnetBoy\_work\epro\SHEET\29352fd497a6489db22a922a94ec8f73\1.esch"
lines=[json.loads(l) for l in open(p,encoding="utf-8").read().splitlines() if l.strip()]
for a in lines[:6]: print(a)
print("...")
n=0
for a in lines:
    if a[0]!="ATTR":
        if n<80: print(a)
        n+=1
print("non-ATTR count",n)
