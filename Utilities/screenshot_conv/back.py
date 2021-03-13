
import os
import sys
import struct
from PIL import Image

def remap_img(data):
    n= []
    
    for y in range(400):
        for x in range(240):
            index = (240 * y + x) * 2
            pix = (data[index + 1] << 8) | data[index]
            b = (pix & 0b1111100000000000) >> (8)
            g = (pix & 0b0000011111100000) >> (3)
            r = (pix & 0b0000000000011111) << (3)
            val = (r << 16) | (g << 8) | b
            n.append(val)
            
    return n
           
filename = sys.argv[1]

img = Image.open(filename)

data = bytes([])
for y in range(400):
    for x in range(240):
        r,g,b = img.getpixel((x,y))

        r >>= 3;
        g >>= 2;
        b >>= 3;

        value = (r << 11) | (g << 5) | b;
    
        data += struct.pack("H", value)

f = open(os.path.basename(filename)[:-4] + ".bin", "wb")
f.write(data)
f.close()

    

