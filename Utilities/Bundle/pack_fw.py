#!/usr/bin/python3

import sys
import random
import struct
import zlib
import os


def read_chunk(bin_path, addr):
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
    
    return chunk


chunks = []

stm_fw_path = os.path.dirname(os.path.realpath(__file__)) + "/../../BB3/Debug/BB3.bin"
chunks.append(read_chunk(stm_fw_path, 0x0))

esp_fw_base_path = os.path.dirname(os.path.realpath(__file__)) + "/../../BB_esp_fw/build/"
esp_chunks_path = os.path.join(esp_fw_base_path, "flash_args")

for line in open(esp_chunks_path, "r").readlines()[1:]:
    addr, bin_path = line.split()
    addr = int(addr, base = 16)
    bin_path = os.path.join(esp_fw_base_path, bin_path)
    
    chunks.append(read_chunk(bin_path, addr))


chunks.sort(key=lambda a: a["addr"])


build = 0xFFFF0000 | random.randint(0, 0xFFFF)
number_of_records = len(chunks)

f = open("strato.fw", "wb")

f.write(struct.pack("<L", build))
f.write(struct.pack("<b", number_of_records))

for i in range(32-5):
    f.write(struct.pack("<b", 0))

for c in chunks:
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
