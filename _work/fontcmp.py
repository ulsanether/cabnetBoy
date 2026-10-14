import sys
sys.path.insert(0, r"D:\GIT\cabnetBoy\_work")
from mkfont2 import raster, preview
import os
cands = [(r"C:\Windows\Fonts\DejaVuSansMono.ttf", 10, 90, False),
         (r"C:\Windows\Fonts\DejaVuSansMono.ttf", 11, 90, False),
         (r"C:\Windows\Fonts\CascadiaMono.ttf", 10, 90, False),
         (r"C:\Windows\Fonts\CascadiaMono.ttf", 11, 90, False),
         (r"C:\Windows\Fonts\DejaVuSansMono-Bold.ttf", 10, 90, False)]
for spec in cands:
    g = raster(*spec)
    name = os.path.basename(spec[0]).split('.')[0] + f"_{spec[1]}"
    preview(g, rf"D:\GIT\cabnetBoy\_work\fp_{name}.png")
    print("ok", name)
