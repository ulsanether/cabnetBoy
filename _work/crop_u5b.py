import sys
sys.path.insert(0, r"D:\GIT\cabnetBoy\_work")
import mcrop
o=r"D:\GIT\cabnetBoy\_work\crops"
mcrop.crop(r"D:\GIT\cabnetBoy\_work\board_top.png", o+r"\u5_find_top.png", 27, -3, 42, -33, 3)
mcrop.crop(r"D:\GIT\cabnetBoy\_work\board_bottom.png", o+r"\u5_find_bot.png", 27, -3, 42, -33, 3)
