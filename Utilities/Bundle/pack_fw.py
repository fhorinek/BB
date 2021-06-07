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
stm_map_path = os.path.dirname(os.path.realpath(__file__)) + "/../../BB3/Debug/BB3.map"
stm_list_path = os.path.dirname(os.path.realpath(__file__)) + "/../../BB3/Debug/BB3.list"

chunks.append(read_chunk(stm_fw_path, 0x0))

esp_fw_base_path = os.path.dirname(os.path.realpath(__file__)) + "/../../BB_esp_fw/build/"
esp_chunks_path = os.path.join(esp_fw_base_path, "flash_args")

for line in open(esp_chunks_path, "r").readlines()[1:]:
    addr, bin_path = line.split()
    addr = int(addr, base = 16)
    bin_path = os.path.join(esp_fw_base_path, bin_path)
    
    chunks.append(read_chunk(bin_path, addr))


chunks.sort(key=lambda a: a["addr"])

try:
    build_number = int(open("build_number", "r").read())
except:
    build_number = 0
    
build_number += 1

open("build_number", "w").write("%u" % build_number)

if len(sys.argv) == 2:
    ch = sys.argv[1]
    if ch not in ["R", "T", "D"]:
        ch = "D"
else:
    ch = "D"
    
build = build_number | ord(ch) << 24

number_of_records = len(chunks)

filename = "%c%07u" % (ch, build_number)

f = open(filename + ".fw", "wb")

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

import os
import shutil

if os.path.exists("strato.fw"):
    os.remove("strato.fw");
    
shutil.copyfile(filename + ".fw", "strato.fw")
shutil.copyfile(stm_map_path, os.path.join("debug", filename + ".map"))
shutil.copyfile(stm_list_path, os.path.join("debug", filename + ".list"))

print("Done")

os.system("git add -A")
os.system("git commit -m \"Release %s\"" % filename)
os.system("read");

