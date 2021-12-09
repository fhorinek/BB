#!/usr/bin/python

def write_file(path, data):
#    print("write_file", path)
    f = open(path, "wb")
    f.write(data)
    f.close()

import struct
import sys
import os

print("unpacking %s", sys.argv[1])


f = open(sys.argv[1], "rb")
header = f.read(32)
build, number_of_records = struct.unpack("<Lb", header[:5])
ch = (build & (0xFF << 24)) >> 24
build = "%c%07u" % (ch, build & ~(0xFF << 24))

print("FW %s (%u chunks)" % (build, number_of_records))

path = [build]
os.mkdir(os.path.join(*path))

old_level = 0
for i in range(number_of_records):
    chunk = f.read(12 + 32)
    addr, size, crc, name = struct.unpack("<LLL32s", chunk)
    name = str(name[:name.index(0)], encoding='ascii')
    if size % 4 != 0:
        size =  size + (4 - size % 4)

    print("  0x%08X\t%10u bytes\tcrc %08X\t%s" % (addr, size, crc, name))

    data = f.read(size)
    
    #dir
    if addr & 0x80000000:
        addr &= ~0x80000000
        index = addr & 0xFFFF
        level = (addr & (0xFFFF << 16)) >> 16
        
        if old_level > level:
            path = path[:-1]
        
        path += [name]
        os.mkdir(os.path.join(*path))
        old_level = level
    
    #file
    elif addr & 0x40000000:
        addr &= ~0x40000000
        index = addr & 0xFFFF
        level = (addr & (0xFFFF << 16)) >> 16
        
        if old_level > level:
            path = path[:-1]
        
        
        write_file(os.path.join(*path, name), data)
        old_level = level        
    #fw
    else:        
        write_file(os.path.join(build, name), data)
        
