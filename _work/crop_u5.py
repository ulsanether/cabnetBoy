import sys
sys.path.insert(0, r"D:\GIT\cabnetBoy\_work")
import importlib, mcrop
importlib.reload(mcrop)
o=r"D:\GIT\cabnetBoy\_work\crops"
mcrop.crop(r"D:\GIT\cabnetBoy\_work\board_bottom.png", o+r"\u5_bot.png", 17, -3, 33, -31, 3)
mcrop.crop(r"D:\GIT\cabnetBoy\_work\board_top.png",    o+r"\u5_top.png", 17, -3, 33, -31, 3)
