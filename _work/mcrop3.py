import sys
sys.path.insert(0,r"D:\GIT\cabnetBoy\_work")
from mcrop import crop
o=r"D:\GIT\cabnetBoy\_work\crops"
crop(r"D:\GIT\cabnetBoy\_work\board_top.png", o+r"\z9_floatpad_top.png", -8, -20, 10, -35, 2.5)
crop(r"D:\GIT\cabnetBoy\_work\board_bottom.png", o+r"\z10_floatpad_bot.png", -8, -20, 10, -35, 2.5)
