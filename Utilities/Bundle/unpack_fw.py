#!/usr/bin/python

def write_file(path, data):
#    print("write_file", path)
    f = open(path, "wb")
    f.write(data)
    f.close()

import struct
import sys
import os

print("unpacking %s" % sys.argv[1])


fil = open(sys.argv[1], "rb")
header = fil.read(32)
build_devel, number_of_records, build_testing, build_release = struct.unpack("<LHHH", header[:10])

if build_release != 0:
    ch = 'R'
elif build_testing != 0:
    ch = 'T'
else:
    ch = 'D'
    
build = "%c.%u.%u.%u" % (ch, build_devel, build_testing, build_release)

print("FW %s (%u chunks)" % (build, number_of_records))

path = [build]
os.mkdir(os.path.join(*path))

old_level = 0
for i in range(number_of_records):
    chunk = fil.read(12 + 32)
    raddr, size, crc, name = struct.unpack("<LLL32s", chunk)
    name = str(name[:name.index(0)], encoding='ascii')
  
    if size % 4 != 0:
        pad = (4 - size % 4)
    else:
        pad = 0

    

    data = fil.read(size)
    __ = fil.read(pad)
    
    addr = raddr    
        
    d = False
    f = False
    #dir
    if addr & 0x80000000:
        addr &= ~0x80000000
        index = addr & 0xFFFF
        level = (addr & (0xFFFF << 16)) >> 16
        d = True
        
        if old_level > level:
            levels_up = old_level - level
            path = path[:-levels_up]
        
        path += [name]
        os.mkdir(os.path.join(*path))
        old_level = level
    
    #file
    elif addr & 0x40000000:
        addr &= ~0x40000000
        index = addr & 0xFFFF
        level = (addr & (0xFFFF << 16)) >> 16
        
        if old_level > level:
            levels_up = old_level - level
            path = path[:-levels_up]
        
        
        write_file(os.path.join(*path, name), data)
        old_level = level        
    #fw
    else:        
        write_file(os.path.join(build, name), data)
        f = True
        
    pad = "*" if f else (("  " * level) + ("[" if d else ""))
    
    print("  0x%08X\t%10u bytes\tcrc %08X\t%s%s%s" % (raddr, size, crc, pad, name, "]" if d else ""))

