import os, re, shutil
src=r"D:\GIT\cabnetBoy\_work\gerber"; dst=r"D:\GIT\cabnetBoy\_work\gerber_fix"
os.makedirs(dst, exist_ok=True)
def num(s):
    neg = s.startswith('-')
    s=s.lstrip('-')
    # 2 integer, 4 decimal, leading zeros omitted
    v=int(s)/10000.0
    return -v if neg else v
for f in os.listdir(src):
    s=os.path.join(src,f); d=os.path.join(dst,f)
    if f.endswith(".DRL"):
        out=[]
        for l in open(s,encoding="utf-8",errors="replace").read().splitlines():
            m=re.match(r'^X(-?\d+)Y(-?\d+)G85X(-?\d+)Y(-?\d+)$', l.strip())
            if m:
                x=(num(m.group(1))+num(m.group(3)))/2; y=(num(m.group(2))+num(m.group(4)))/2
                out.append(f"X{int(round(x*10000))}Y{int(round(y*10000))}")
            else:
                out.append(l.replace('INCH,LZ','INCH,LZ,00.0000') if l.strip()=='INCH,LZ' else l)
        open(d,"w",encoding="utf-8").write("\n".join(out)+"\n")
    else:
        shutil.copy2(s,d)
print("ok")

