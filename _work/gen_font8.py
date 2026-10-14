import sys, os
sys.path.insert(0, r"D:\GIT\cabnetBoy\_work")
from mkfont2 import raster

FONT = r"C:\Windows\Fonts\DejaVuSansMono.ttf"
OUT = r"D:\GIT\cabnetBoy\software\cabnetBoyRogue\src\font8.h"

g = raster(FONT, 11, 90, False)
g = g[:64]            # ASCII 32..95 only (lowercase folds to uppercase)

lines = []
lines.append("// ============================================================================")
lines.append("// font8.h - 8x8 bitmap font for ASCII 32..95 (space..underscore)")
lines.append("//")
lines.append("// Auto-generated from DejaVu Sans Mono (Bitstream Vera / DejaVu licence,")
lines.append("// permissive) rasterised at 11 px and thresholded into an 8x8 cell.")
lines.append("// One glyph = 8 bytes, one byte per pixel row, MSB = leftmost pixel.")
lines.append("// Lower case is folded to upper case by the renderer (authentic roguelike look).")
lines.append("// ============================================================================")
lines.append("#pragma once")
lines.append("")
lines.append("#include <stdint.h>")
lines.append("")
lines.append("#define FONT_FIRST 32")
lines.append("#define FONT_LAST  95")
lines.append("")
lines.append("const uint8_t FONT8[(FONT_LAST - FONT_FIRST + 1) * 8] = {")
for ci, gg in enumerate(g):
    c = ci + 32
    disp = chr(c)
    if disp == "\\":
        disp = "backslash"
    elif disp == "'":
        disp = "quote"
    lines.append(f"  {', '.join('0x%02X' % b for b in gg)},   // '{disp}' 0x{c:02X}")
lines.append("};")
lines.append("")

open(OUT, "w", encoding="utf-8", newline="\n").write("\n".join(lines) + "\n")

# sanity preview of a few glyphs
for ch in "@#.>+!$ARZ0123456789":
    i = ord(ch) - 32
    print(f"--- '{ch}' ---")
    for r in g[i]:
        print("".join("#" if r & (1 << (7 - x)) else "." for x in range(8)))
print("\nwrote", OUT, os.path.getsize(OUT), "bytes")
