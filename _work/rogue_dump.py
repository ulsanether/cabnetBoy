import sys
sys.path.insert(0, r"D:\GIT\cabnetBoy\_work")
import rogue_preview as R
g = R.G(1)
g.map=[R.TER_WALL]*(R.MAP_W*R.MAP_H)
def room(x0,y0,x1,y1,t=R.TER_FLOOR):
    for y in range(y0,y1+1):
        for x in range(x0,x1+1): g.mset(x,y,t)
room(2,2,9,7); room(12,9,19,14)
for x in range(7,15): g.mset(x,4,R.TER_FLOOR)
for y in range(4,12): g.mset(14,y,R.TER_FLOOR)
g.mset(14,4,R.TER_DOOR); g.mset(19,14,R.TER_STAIR)
g.px,g.py,g.facing=5,5,1
g.mons=[[7,4,4,0],[8,6,6,1],[13,9,5,2]]
g.items=[[3,3,0],[6,6,1]]
g.fov()
sym={0:'.',1:'#',2:'+',3:'>'}
cx=max(0,min(g.px-8,R.MAP_W-16)); cy=max(0,min(g.py-3,R.MAP_H-6))
print(f"camera=({cx},{cy})  player=({g.px},{g.py})")
print("+"+"-"*16+"+")
for row in range(R.VIEW_H):
    line=""
    for col in range(R.VIEW_W):
        v=g.cell_code(cx+col, cy+row)
        if v<32: line += {0:' '}.get(v, "###"[v-1] if v<4 else 'P')
        else: line += chr(v)
    print("|"+line+"|")
print("+"+"-"*16+"+")
print("traits: raw map rows 2..7 (x0..15)")
for y in range(2,8):
    print("".join(".#+>"[g.mget(x,y)] for x in range(0,16)), "   vis:", "".join(str(g.bget(g.vis,x,y)) for x in range(0,16)))
