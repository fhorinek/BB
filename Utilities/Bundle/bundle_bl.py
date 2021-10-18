#!/usr/bin/python3

import sys
import random
import struct
import zlib
import os
import shutil

try:
    build_number = int(open("bl_build_number", "r").read())
except:
    build_number = 0
    
build_number += 1

open("bl_build_number", "w").write("%u" % build_number)

bl_base_path = os.path.dirname(os.path.realpath(__file__)) + "/../../BB3_loader"
bl_fw_file = os.path.join(bl_base_path, "Release", "BB3_loader.bin")

data = open(bl_fw_file, "rb").read()
#align data to 32b
if len(data) % 4 != 0:
    data += bytes([0] * (4 - (len(data) % 4)))
size = len(data)


crc = zlib.crc32(data)

filename = "bootloader.fw"
f = open(filename, "wb")
#number
f.write(struct.pack("<L", build_number))
#lenght
f.write(struct.pack("<L", size))
#reserved
f.write(struct.pack("<L", 0))
#crc
f.write(struct.pack("<L", crc))

f.write(data)
f.close()

print("Bootloader bundle")
print(" number %u" % build_number)
print(" size   %u" % size)
print(" crc    %08X" % crc)

stm_base_path = os.path.dirname(os.path.realpath(__file__)) + "/../../BB3/"
bl_assets_path = os.path.join(stm_base_path, "Assets", filename)

shutil.copyfile(filename, bl_assets_path)

