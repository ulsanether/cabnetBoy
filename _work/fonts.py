import os
from PIL import Image, ImageDraw, ImageFont
cands = [r"C:\Windows\Fonts\consola.ttf", r"C:\Windows\Fonts\lucon.ttf",
         r"C:\Windows\Fonts\cour.ttf", r"C:\Windows\Fonts\tahoma.ttf",
         r"C:\Windows\Fonts\arial.ttf"]
for c in cands:
    print(os.path.exists(c), c)
