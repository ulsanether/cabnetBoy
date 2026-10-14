import re, os, collections
d=r"D:\GIT\cabnetBoy\_work\gerber"
for fn in sorted(os.listdir(d)):
    if not (fn.endswith(".GTL") or fn.endswith(".GBL") or fn.endswith(".GTO") or fn.endswith(".GBO") or fn.endswith(".GTS") or fn.endswith(".GBS") or fn.endswith(".GTP") or fn.endswith(".GBP") or fn.endswith(".GKO") or fn.endswith(".GDL")):
        continue
    txt=open(os.path.join(d,fn),encoding="utf-8",errors="replace").read()
    aps=re.findall(r'%ADD(\d+)([A-Z]+),([^*%]*)\*%',txt)
    fmt=re.findall(r'%FS([A-Z]+)X(\d)(\d)Y(\d)(\d)\*%',txt)
    mo=re.findall(r'%MO(\w+)\*%',txt)
    cmds=collections.Counter(re.findall(r'\b(G0[123]|G36|G37|G74|G75)\*',txt))
    # which apertures used for D01/D03
    cur=None; use=collections.Counter()
    for line in txt.splitlines():
        m=re.match(r'^D(\d+)\*$',line.strip())
        if m: cur=m.group(1); continue
        m=re.match(r'^(?:X-?\d+)?(?:Y-?\d+)?D0([13])\*$',line.strip())
        if m: use[(cur,m.group(1))]+=1
    print("="*70)
    print(fn, "FS",fmt,"MO",mo)
    print("  apertures:", [ (a,b,c) for a,b,c in aps ])
    print("  cmd:",dict(cmds))
    print("  usage:",dict(use))
