import sys, os
sys.path.insert(0, r"D:\GIT\cabnetBoy\_work")
import rogue_preview as R
from PIL import Image, ImageDraw
PLAYER=[0x3C,0x42,0x42,0x5A,0x5A,0x42,0x42,0x3C]
F25=[0x88,0x22,0x88,0x22,0x88,0x22,0x88,0x22]
BLACK=[0]*8
W50=[0xAA,0x55,0xAA,0x55,0xAA,0x55,0xAA,0x55]
WEDGE=[0xFF,0x80,0x80,0x80,0x80,0x80,0x80,0x80]   # heavy top edge
WEDGE2=[0xFF,0x81,0x81,0x81,0x81,0x81,0x81,0xFF]  # box outline

def cell(g,x,y,face):
    c=g.cell_code(x,y)
    if c!=2: return c
    # wall: if any 4-neighbour is seen floor, use the "face" tile
    for dx,dy in ((0,-1),(0,1),(-1,0),(1,0)):
        nx,ny=x+dx,y+dy
        if 0<=nx<R.MAP_W and 0<=ny<R.MAP_H and g.bget(g.seen,nx,ny):
            if g.mget(nx,ny) in (1,2,3):
                return face
    return 2

def render(g,face_tile,msg):
    img=Image.new("1",(128,64),0); px=img.load()
    R.TILES=[BLACK,F25,BLACK,[0]*8,PLAYER,face_tile]
    for page in range(8):
        if page<R.VIEW_H:
            cx=max(0,min(g.px-8,R.MAP_W-16)); cy=max(0,min(g.py-3,R.MAP_H-6))
            cells=[cell(g,cx+c,cy+page,5) for c in range(16)]
        else:
            t=(g.status() if page==6 else msg); cells=[ord(ch.upper()) for ch in (t+" "*16)[:16]]
        for col,val in enumerate(cells):
            b=R.glyph_of(val)
            for x in range(8):
                for y in range(8):
                    if b[y]&(0x80>>x): px[col*8+x,page*8+y]=1
    return img

g=R.G(0x7777); g.depth=4; g.gen_level(); g.fov()
names=["no face","face=dither50","face=topedge","face=box"]
faces=[None,W50,WEDGE,WEDGE2]
W,H=128*3,64*3
out=Image.new("RGB",(W*len(names),H+14),(30,30,30)); d=ImageDraw.Draw(out)
for i,(n,f) in enumerate(zip(names,faces)):
    if f is None:
        R.TILES=[BLACK,F25,BLACK,[0]*8,PLAYER]
        im=R.render(g,"OUTLINE")
    else:
        im=render(g,f,"OUTLINE")
    out.paste(im.convert("RGB").resize((W,H),Image.NEAREST),(i*W,14))
    d.text((i*W+4,2),n,fill=(255,255,0))
p=r"D:\GIT\cabnetBoy\_work\rogue\outline_compare.png"; out.save(p); print("wrote",p)
