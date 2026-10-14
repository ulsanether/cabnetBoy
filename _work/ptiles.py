import sys, re, os
sys.path.insert(0, r"D:\GIT\cabnetBoy\_work")
import rogue_preview as R
from PIL import Image
names=["N","E","S","W"]
S=24
out=Image.new("RGB",(4*9*S, 9*S),(20,20,20))
for i in range(4):
    t=R.TILES[4+i]
    for y in range(8):
        for x in range(8):
            if t[y]&(0x80>>x):
                for dy in range(S):
                    for dx in range(S):
                        out.putpixel((i*9*S+x*S+dx, y*S+dy),(255,255,255))
p=r"D:\GIT\cabnetBoy\_work\rogue\player_tiles.png"
out.save(p); print("wrote",p)
print("N",[hex(v) for v in R.TILES[4]])
print("E",[hex(v) for v in R.TILES[5]])
print("S",[hex(v) for v in R.TILES[6]])
print("W",[hex(v) for v in R.TILES[7]])
