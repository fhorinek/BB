#!/usr/bin/python

import sys
import os
import common


path = sys.argv[1]
files = open(path, "r").read().split("\n")
name = os.path.basename(path.split(".")[0])
dst_name = name + ".zip"
dst = os.path.join(common.target_dir_step5, dst_name)
script = os.path.join(common.target_dir_step5, dst_name)
os.makedirs(common.target_dir_step5, exist_ok = True)

agl = ""

for f in files:
    lon, lat = common.filename_to_lon_lat(f)
    common.init_vars(lon, lat)

    tile = os.path.join(common.target_dir_step4, f + ".MAP")
    if os.path.exists(tile):
        os.system("zip -g -j %s %s" % (dst, tile))
        
        atile = f + ".HGT"
        agl += ("zip -g -j $DST/%s %s\n" % (name + "_agl.zip", atile))
    
open(os.path.join(common.target_dir_step5, name + "_agl.sh"), "w").write(agl)
