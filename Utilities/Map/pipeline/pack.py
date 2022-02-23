#!/usr/bin/python

import sys
import os
import common


path = os.path.join(common.target_dir_countries, sys.argv[1] + ".list")
files = open(path, "r").read().split("\n")
name = os.path.basename(path.split(".")[0])
dst_map = os.path.join(common.target_dir_step5, name + "_map.zip")
dst_agl = os.path.join(common.target_dir_step5, name + "_agl.zip")
os.makedirs(common.target_dir_step5, exist_ok = True)

for f in files:
    lon, lat = common.filename_to_lon_lat(f)
    common.init_vars(lon, lat)

    tile = os.path.join(common.target_dir_step4, f + ".MAP")
    if os.path.exists(tile):
        ret = os.system("zip -g -j '%s' '%s'" % (dst_map, tile))
        if ret != 0:
            sys.exit(ret)
        
    tile = os.path.join(common.hgt_path, f + ".HGT")
    if os.path.exists(tile):
        ret = os.system("zip -g -j '%s' '%s'" % (dst_agl, tile))
        if ret != 0:
            sys.exit(ret)
    

