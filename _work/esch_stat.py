import json, collections
p=r"D:\GIT\cabnetBoy\_work\epro\SHEET\29352fd497a6489db22a922a94ec8f73\1.esch"
lines=open(p,encoding="utf-8").read().splitlines()
c=collections.Counter()
objs=[]
for l in lines:
    l=l.strip()
    if not l: continue
    a=json.loads(l)
    c[a[0]]+=1
    objs.append(a)
for k,v in c.most_common(): print(f"{v:6d}  {k}")
