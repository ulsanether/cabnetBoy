import gerbonara, os, glob, pymupdf
d=r"D:\GIT\cabnetBoy\_work\gerber"
stack = gerbonara.LayerStack.open_dir(d)
print(stack)
print([l for l in dir(stack) if not l.startswith('_')])
