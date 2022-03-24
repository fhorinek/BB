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
import subprocess

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
                    help="Increase build number, tag release, create new branch if needed",
                    action="store_true")
parser.add_argument("-c", "--channel", default="A", choices=['R', 'T', 'D', 'A'],
                    help="Select the release channel for this firmware out of (Release, Test, Debug, Auto)")
args = parser.parse_args()

active_branch = subprocess.check_output(["git", "branch", "--show-current"]).decode().split()[0]

print("Active branch is %s" % active_branch)

on_master = active_branch == "master"
on_testing = active_branch.find("testing_") == 0
on_release = active_branch.find("release_") == 0
on_devel = not on_testing and not on_release

branch_numbers = active_branch.replace("testing_", "").replace("release_", "").split(".")

if args.channel == 'A':
    if on_release:
        args.channel = 'R'
    if on_testing:
        args.channel = 'T'
    if on_devel:
        args.channel = 'D'

#Sanity checks
if args.publish:
    if args.channel == "D" and not on_master:
        print("Error: Please do not publish devel if not on master branch")
        sys.exit(-1)
    if args.channel == "T" and (not on_testing and not on_devel):
        print("Error: Please do not publish testing if not on testing or devel branch")
        sys.exit(-1)
    if args.channel == "R" and (not on_testing and not on_release):
        print("Error: Please do not publish release if not on release or testin branch")
        sys.exit(-1)

if args.publish:
    not_staged = os.system("git diff --exit-code")
    not_commited = os.system("git diff --cached --exit-code")
    
    if not_staged != 0 or not_commited != 0:
        print("Error: Commit your changes first!")
        sys.exit(-1)

#STM firmware
chunks.append(read_chunk(stm_bin_file, 0x0))

#add assets
add_files(stm_assets_path, 0)


#ESP
esp_fw_base_path = os.path.dirname(os.path.realpath(__file__)) + "/../../BB_esp_fw/build/"
esp_chunks_path = os.path.join(esp_fw_base_path, "flash_args")

if not os.path.isfile(esp_chunks_path):
    print("BB_esp_fw is missing. Please either compile this manually or follow instructions on https://github.com/fhorinek/BB/blob/master/Dev/Setup.md#workaround-problems-installing for a workaround.")
    sys.exit(1)

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
    build_devel = int(open("build_devel", "r").read())
except:
    build_devel = 0

try:    
    build_testing = int(open("build_testing", "r").read())
except:
    build_testing = 0

try:
    build_release = int(open("build_release", "r").read())
except:
    build_release = 0
    
if on_testing:
    if build_devel != int(branch_numbers[0]):
        print("Build number is different! file %u.x.x != branch %u.x.x" % (build_devel, int(branch_numbers[0])))
        sys.exit(-1)

if on_release:
    if build_devel != int(branch_numbers[0]) or build_testing != int(branch_numbers[1]):
        print("Build number is different! file %u.%u.x != branch %u.%u.x" % (build_devel, build_testing, int(branch_numbers[0]), int(branch_numbers[1])))
        sys.exit(-1)

if args.publish:
    if args.channel == "R":
        if build_release == 0:
            os.system("git checkout -b release_%u.%u.x" % (build_devel, build_testing))
    
        build_release += 1
        open("build_release", "w").write("%u" % build_release)
    elif args.channel == "T":
        build_release = 0
        if build_testing == 0:
            os.system("git checkout -b testing_%u.x.x" % build_devel)
            
        build_testing += 1
        open("build_testing", "w").write("%u" % build_testing)
        open("build_release", "w").write("%u" % build_release)
    else:
        build_devel += 1
        build_release = 0
        build_testing = 0
        open("build_devel", "w").write("%u" % build_devel)
        open("build_testing", "w").write("%u" % build_testing)
        open("build_release", "w").write("%u" % build_release)

number_of_records = len(chunks)

if os.path.exists("strato.fw"):
    os.remove("strato.fw");

f = open("strato.fw", "wb")

f.write(struct.pack("<L", build_devel))
f.write(struct.pack("<b", number_of_records))
f.write(struct.pack("<H", build_testing))
f.write(struct.pack("<H", build_release))

#pad rest of the app header
for i in range(32 - (4 + 1 + 2 + 2)):
    f.write(struct.pack("<b", 0))

for c in chunks:
    f.write(struct.pack("<L", c["addr"]))
    f.write(struct.pack("<L", c["size"]))
    f.write(struct.pack("<L", c["crc"]))
    f.write(c["name"])        

    f.write(c["data"])

f.close()

build = "%c.%u.%u.%u" % (args.channel, build_devel, build_testing, build_release)

if args.publish:
    folder = os.path.dirname(os.path.realpath(__file__)) + "/build/%s" % (build)

    os.makedirs(folder, exist_ok=True)
    shutil.copyfile("strato.fw", os.path.join(folder, "strato.fw"))
    shutil.copyfile("strato.fw", os.path.join(folder, "%s.fw" % build))
    shutil.copyfile(stm_map_file, os.path.join(folder, "BB3.map"))
    shutil.copyfile(stm_list_file, os.path.join(folder, "BB3.list"))

    os.system("git add -A")
    os.system("git commit -m 'Added binaries for relese %s'" % build)
    os.system("git tag '%s'" % build)

print("Done for build " + build)


