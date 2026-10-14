import json, collections, os
p=r"D:\GIT\cabnetBoy\_work\epro\PCB\15bb077a273c4ed1a85f056a4b433f5d.epcb"
lines=open(p,encoding="utf-8").read().splitlines()
c=collections.Counter()
bad=0
for i,l in enumerate(lines):
    l=l.strip()
    if not l: continue
    try:
        a=json.loads(l)
    except Exception as e:
        bad+=1; continue
    c[a[0]]+=1
print("total lines",len(lines),"bad",bad)
for k,v in c.most_common(60): print(f"{v:6d}  {k}")
