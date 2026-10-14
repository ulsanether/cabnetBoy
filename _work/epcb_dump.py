import json
p=r"D:\GIT\cabnetBoy\_work\epro\PCB\15bb077a273c4ed1a85f056a4b433f5d.epcb"
L=[json.loads(l) for l in open(p,encoding="utf-8").read().splitlines() if l.strip()]
print("--- first 40 ---")
for a in L[:40]: print(json.dumps(a,ensure_ascii=False)[:300])
print("--- samples ---")
seen=set()
for a in L:
    t=a[0]
    if t in ("COMPONENT","NET","PAD_NET","LINE","VIA","RULE","RULE_SELECTOR","POUR","POURED","PRIMITIVE","DIMENSION","POLY","STRING","ATTR","PROP","PAD","LAYER_PHYS"):
        if seen.get(t,0)<3 if isinstance(seen,dict) else False: pass
print("--- COMPONENT ---")
for a in L:
    if a[0]=="COMPONENT": print(json.dumps(a,ensure_ascii=False)[:400])
