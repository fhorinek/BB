#!/usr/bin/python3
#
# This program combines all parts of the Strato firmware into
# "strato.fw" and copies it to Utilities/Bundle/bundle/ into a
# directory with the build number.
#
# This script is used after compiling the parts to create the final
# firmware file.
#
# Use the option "--help" for an explanation.
#

import sys
import random
import struct
import zlib
import os
import shutil
import argparse
import tempfile

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
        taddr = addr & ~0x80000000
        level = (taddr & (0xFFFF << 16)) >> 16
        d = True
    
    #file
    elif addr & 0x40000000:
        taddr = addr & ~0x40000000
        level = (taddr & (0xFFFF << 16)) >> 16
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

def work_release_note(release_note_filename):
    if args.channel != 'R':
        message = "WARNING!\n\nYou are using a preview version as the current firmware.\nThis brings in new features, but it may be unstable.\nPlease switch to the release channel for stable versions.\n\nTurning on development mode and \"Logging to file\" for possible bug reports."
        with open(release_note_filename, 'r+') as f:
            content = f.read()
            f.seek(0, 0)
            f.write(message + content)
                      
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

            
parser = argparse.ArgumentParser(description='Pack parts to create a Strato firmware.')
parser.add_argument("-p", "--publish",
                    help="Publish the firmware (=git add,commit)",
                    action="store_true")
parser.add_argument("-i", "--ignore",
                    help="Do not increase build number",
                    action="store_true")
parser.add_argument("-c", "--channel", default="D", choices=['R', 'T', 'D'],
                    help="Select the release channel for this firmware out of (Release, Test, Debug)")
args = parser.parse_args()

#STM firmware
chunks.append(read_chunk(stm_bin_file, 0x0))

# Copy Assets to a temporary directory and change release notes there
# Add them to chunks.
dirpath = tempfile.mkdtemp()
shutil.copytree(stm_assets_path, dirpath, dirs_exist_ok=True)
work_release_note(os.path.join(dirpath, "release_note.txt"))

#add assets
add_files(dirpath, 0)

shutil.rmtree(dirpath)

#ESP
esp_fw_base_path = os.path.dirname(os.path.realpath(__file__)) + "/../../BB_esp_fw/build/"
esp_chunks_path = os.path.join(esp_fw_base_path, "flash_args")

esp_chunks = []

for line in open(esp_chunks_path, "r").readlines()[1:]:
    addr, bin_path = line.split()
    addr = int(addr, base = 16)
    bin_path = os.path.join(esp_fw_base_path, bin_path)
    esp_chunks.append((addr, bin_path))
    
esp_chunks.sort(key = lambda a: a[0])    

for record in esp_chunks:
    addr, bin_path = record
    chunks.append(read_chunk(bin_path, addr))

try:
    build_number = int(open("build_number", "r").read())
except:
    build_number = 0

if args.ignore != True:
    build_number += 1

open("build_number", "w").write("%u" % build_number)

build = build_number | ord(args.channel) << 24

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

build = "%c%07u" % (args.channel, build_number)
folder = os.path.dirname(os.path.realpath(__file__)) + "/build/%s" % (build)
    
os.makedirs(folder, exist_ok=True)
shutil.copyfile("strato.fw", os.path.join(folder, "strato.fw"))
shutil.copyfile("strato.fw", os.path.join(folder, "%s.fw" % build))
shutil.copyfile(stm_map_file, os.path.join(folder, "BB3.map"))
shutil.copyfile(stm_list_file, os.path.join(folder, "BB3.list"))

if args.publish:
    print("Creating Snapshot Build %s" % build)
    os.system("git add -A")
    os.system("git commit -m \"Snapshot build %s\"" % build)

print("Done for build " + build)


