#!/usr/bin/python3

import sys
import random
import struct
import zlib

fname = sys.argv[1]

data = open(fname, "rb").read()
crc = zlib.crc32(data)
size = len(data)
build = 0xFFFF0000 | random.randint(0,0xFFFF)

print("Packing %s" % fname)
print(" build: %08X, %u" % (build, build))
print(" size:  %08X, %u" % (size, size))
print(" crc:   %08X" % (crc))

f = open("boombox.fw", "wb")

f.write(struct.pack("<L", build))
f.write(struct.pack("<L", size))
f.write(struct.pack("<L", crc))

for i in range(20):
    f.write(struct.pack("<b", 0))

f.write(data)

f.close()
