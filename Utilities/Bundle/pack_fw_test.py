#!/usr/bin/python3

import sys
import random
import struct
import zlib
import os
import shutil


name_max_len = 32

def read_chunk(bin_path, addr):

    if os.path.isdir(bin_path):
        data = bytes([])
    else:
        data = open(bin_path, "rb").read()

    size = len(data)
    #align data to 32b
    if len(data) % 4 != 0:
        data += bytes([0] * (4 - (len(data) % 4)))
        
    name = bytes(os.path.basename(bin_path), encoding='ascii')
    name = name[0:name_max_len - 1]
    name += bytes([0] * (name_max_len - len(name)))


    if addr == 0:
        #no meta info for STM fw
        crc = zlib.crc32(data)
    else:
        meta = name + struct.pack("<LL", addr, size) 
        crc = zlib.crc32(data + meta)

    d = False
    f = False
    #dir
    if addr & 0x80000000:
        addr &= ~0x80000000
        level = (addr & (0xFFFF << 16)) >> 16
        d = True
    
    #file
    elif addr & 0x40000000:
        addr &= ~0x40000000
        level = (addr & (0xFFFF << 16)) >> 16
    else:
        level = 0
        f = True

    pad = "*" if f else (("  " * level) + ("[" if d else ""))
    

    print("  0x%08X\t%10u bytes\tcrc %08X\t%s%s%s" % (addr, size, crc, pad, str(name, encoding='ascii'), "]" if d else ""))
    
    chunk = {
        "name": name,
        "size": size,
        "crc": crc,
        "addr": addr,
        "data": data
    }
    
    return chunk


chunks = []

#STM
stm_base_path = os.path.dirname(os.path.realpath(__file__)) + "/../../BB3/"
stm_assets_path = os.path.join(stm_base_path, "Assets")
stm_fw_path = os.path.join(stm_base_path, "Release")
stm_bin_file = os.path.join(stm_fw_path, "BB3.bin")
stm_map_file = os.path.join(stm_fw_path, "BB3.map")
stm_list_file = os.path.join(stm_fw_path, "BB3.list")

# assets
def add_files(path, level):
    #print("add_files", path, level)
    index = 0
    #files
    for file in os.listdir(path):
        if file[0] == ".":
            continue
        if file[-1] == "~":
            continue

        chunk = os.path.join(path, file)
        if os.path.isdir(chunk):
            continue
        
        addr = 0x40000000 | index | level << 16
        index += 1
        chunks.append(read_chunk(chunk, addr))

    #dirs
    for file in os.listdir(path):
        if file[0] == ".":
            continue

        chunk = os.path.join(path, file)
        if not os.path.isdir(chunk):
            continue
            
        addr = 0x80000000 | index | level << 16
        index += 1        
        chunks.append(read_chunk(chunk, addr))
        add_files(chunk, level + 1)

            


#STM firmware
chunks.append(read_chunk(stm_bin_file, 0x0))

#add assets
add_files(stm_assets_path, 0)


#ESP
esp_fw_base_path = os.path.dirname(os.path.realpath(__file__)) + "/../../BB_esp_fw/build/"
esp_chunks_path = os.path.join(esp_fw_base_path, "flash_args")

for line in open(esp_chunks_path, "r").readlines()[1:]:
    addr, bin_path = line.split()
    addr = int(addr, base = 16)
    bin_path = os.path.join(esp_fw_base_path, bin_path)
    
    chunks.append(read_chunk(bin_path, addr))

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

if os.path.exists("strato.fw"):
    os.remove("strato.fw");

f = open("strato.fw", "wb")

f.write(struct.pack("<L", build))
f.write(struct.pack("<b", number_of_records))

for i in range(32-5):
    f.write(struct.pack("<b", 0))

for c in chunks:
    f.write(struct.pack("<L", c["addr"]))
    f.write(struct.pack("<L", c["size"]))
    f.write(struct.pack("<L", c["crc"]))
    f.write(c["name"])        

    f.write(c["data"])

f.close()

build = "%c%07u" % (ch, build_number)
folder = os.path.dirname(os.path.realpath(__file__)) + "/build/%s" % (build)

os.mkdir(folder)
shutil.copyfile("strato.fw", os.path.join(folder, "strato.fw"))
shutil.copyfile("strato.fw", os.path.join(folder, "%s.fw" % build))
shutil.copyfile(stm_map_file, os.path.join(folder, "BB3.map"))
shutil.copyfile(stm_list_file, os.path.join(folder, "BB3.list"))

print("Done")


