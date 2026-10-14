from PIL import Image
import os
src=r"D:\GIT\cabnetBoy\_work\board_top.png"
im=Image.open(src); W,H=im.size
print(W,H)
regs={
 "t1_u6_power":(0.0,0.0,0.55,0.30),
 "t2_lcdhdr":(0.20,0.03,0.75,0.15),
 "t3_u5":(0.60,0.03,1.0,0.32),
 "t4_bottom_left":(0.0,0.60,0.55,1.0),
 "t5_handler":(0.40,0.60,0.90,0.95),
}
for k,(x0,y0,x1,y1) in regs.items():
    c=im.crop((int(x0*W),int(y0*H),int(x1*W),int(y1*H)))
    c=c.resize((c.width*2,c.height*2), Image.LANCZOS)
    c.save(rf"D:\GIT\cabnetBoy\_work\crops\{k}.png"); print(k,c.size)
