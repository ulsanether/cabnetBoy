import struct, sys
p=r"D:\GIT\cabnetBoy\software\cabnetBoy\.pio\build\CH32V003\firmware.elf"
d=open(p,'rb').read()
# ELF32 header
assert d[:4]==b'\x7fELF'
e_shoff=struct.unpack_from('<I',d,0x20)[0]
e_shentsize=struct.unpack_from('<H',d,0x2e)[0]
e_shnum=struct.unpack_from('<H',d,0x30)[0]
e_shstrndx=struct.unpack_from('<H',d,0x32)[0]
def sh(i):
    o=e_shoff+i*e_shentsize
    name,typ,flags,addr,off,size,link,info,align,entsize=struct.unpack_from('<IIIIIIIIII',d,o)
    return dict(name=name,typ=typ,flags=flags,addr=addr,off=off,size=size)
strtab_off=sh(e_shstrndx)['off']
def nm(x):
    e=d.index(b'\0',strtab_off+x)
    return d[strtab_off+x:e].decode()
tot_flash=0; tot_ram=0
print(f"{'section':22} {'addr':>10} {'size':>8}")
for i in range(e_shnum):
    s=sh(i); n=nm(s['name'])
    if s['typ']==8: continue
    if s['size']==0: continue
    print(f"{n:22} {s['addr']:>10} {s['size']:>8}")
    if s['flags'] & 0x2:  # ALLOC
        if s['addr'] < 0x20000000:
            tot_flash += s['size']
        else:
            tot_ram += s['size']
print()
print("FLASH (alloc, <0x20000000):", tot_flash, "bytes  / 16384")
print("RAM   (alloc, >=0x20000000):", tot_ram, "bytes  / 2048")
