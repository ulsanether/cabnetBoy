import struct
p=r"D:\GIT\cabnetBoy\software\cabnetBoy\.pio\build\CH32V003\firmware.elf"
d=open(p,'rb').read()
e_shoff=struct.unpack_from('<I',d,0x20)[0]
e_shentsize=struct.unpack_from('<H',d,0x2e)[0]
e_shnum=struct.unpack_from('<H',d,0x30)[0]
e_shstrndx=struct.unpack_from('<H',d,0x32)[0]
def sh(i):
    o=e_shoff+i*e_shentsize
    return struct.unpack_from('<IIIIIIIIII',d,o)
strtab_off=sh(e_shstrndx)[4]
def nm(x):
    e=d.index(b'\0',strtab_off+x); return d[strtab_off+x:e].decode()
flash=ram=0
print(f"{'section':22}{'type':>6}{'flags':>7}{'addr':>12}{'size':>8}")
for i in range(e_shnum):
    name,typ,flags,addr,off,size = sh(i)[:6]
    n=nm(name)
    if size==0: continue
    print(f"{n:22}{typ:>6}{flags:>7}{addr:>12}{size:>8}")
    if flags & 0x2:
        if 0x20000000 <= addr < 0x20010000: ram += size
        else: flash += size
print()
print("FLASH used:",flash,"/ 16384  (free", 16384-flash, ")")
print("RAM   used:",ram,"/ 2048  (free", 2048-ram, ")")
