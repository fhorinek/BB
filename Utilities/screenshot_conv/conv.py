
import os
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
           
pth = "in/"
raws = os.listdir(pth)
os.system("rm -rf out/*");

for raw in raws:
    print(raw)
    
    img = Image.new('RGB', (240, 400))
    
    f = open(pth + raw, "rb")
    data = f.read()
    f.close()
    
    print("data len", len(data))
    data = remap_img(data)
    
    img.putdata(data)
    img.save("out/%s.png" % raw)
    
#print("Converting to video")
#os.system("rm -rf out.avi");
#os.system("ffmpeg -r 10 -i out/%08d.png -q:v 1 out.avi")
#print "done"
