#!/usr/bin/python3

import sys
import random
import struct
import zlib
import os

base_path = os.path.dirname(os.path.realpath(__file__)) + "/../../build/"
args_path = os.path.join(base_path, "flash_args")
args = []

print("Reading %s" % args_path)
for line in open(args_path, "r").readlines()[1:]:
    addr, bin_path = line.split()
    addr = int(addr, base = 16)
    bin_path = os.path.join(base_path, bin_path)
    data = open(bin_path, "rb").read()
    crc = zlib.crc32(data)
    size = len(data)
    name = os.path.basename(bin_path).split(".")[0]
    print("  0x%X\t%u bytes\tcrc %08X\t%s" % (addr, size, crc, name))
    
    chunk = {
        "name": bytes(name, encoding="ascii"),
        "size": size,
        "crc": crc,
        "addr": addr,
        "data": data
    }
    
    args.append(chunk)


args.sort(key=lambda a: a["addr"])


build = 0xFFFF0000 | random.randint(0, 0xFFFF)
number_of_records = len(args)

f = open("esp.fw", "wb")

f.write(struct.pack("<L", build))
f.write(struct.pack("<b", number_of_records))

for i in range(32-5):
    f.write(struct.pack("<b", 0))

for c in args:
    f.write(struct.pack("<L", c["addr"]))
    f.write(struct.pack("<L", c["size"]))
    f.write(struct.pack("<L", c["crc"]))
    for i in range(16):
        if i < len(c["name"]):
            f.write(struct.pack("<b", c["name"][i]))        
        else:    
            f.write(struct.pack("<b", 0))        
        
    f.write(c["data"])

f.close()
print("Done")
