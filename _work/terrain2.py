import sys, os
sys.path.insert(0, r"D:\GIT\cabnetBoy\_work")
import rogue_preview as R
from PIL import Image, ImageDraw
PLAYER=[0x3C,0x42,0x42,0x5A,0x5A,0x42,0x42,0x3C]
F25=[0x88,0x22,0x88,0x22,0x88,0x22,0x88,0x22]
F50=[0xAA,0x55,0xAA,0x55,0xAA,0x55,0xAA,0x55]
BLACK=[0]*8
CORNER=[0x80,0,0,0,0,0,0,0x01]
TWO=[0x80,0,0x08,0,0x80,0,0x08,0]
VAR={"a f25/black":(F25,BLACK),"b f25/corner":(F25,CORNER),"c f50/black":(F50,BLACK),"d f25/two":(F25,TWO)}
g=R.G(0x7777); g.depth=4; g.gen_level(); g.fov()
W,H=128*3,64*3
out=Image.new("RGB",(W*len(VAR),H+14),(30,30,30)); d=ImageDraw.Draw(out)
for i,(n,(f,w)) in enumerate(VAR.items()):
    R.TILES=[BLACK,f,w,[0]*8,PLAYER]
    im=R.render(g,"TERRAIN 2")
    out.paste(im.convert("RGB").resize((W,H),Image.NEAREST),(i*W,14))
    d.text((i*W+4,2),n,fill=(255,255,0))
p=r"D:\GIT\cabnetBoy\_work\rogue\terrain_compare2.png"; out.save(p); print("wrote",p)
