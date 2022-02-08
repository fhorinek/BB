#!/usr/bin/python

import sys
import os
import common

verbose = False

if (len(sys.argv) == 2):
    countries = [sys.argv[1] + ".list"]
    verbose = True
else:
    countries = os.listdir(common.target_dir_countries)
    

lines = []
for c in countries:
    path = os.path.join(common.target_dir_countries, c)
    tiles = open(path, "r").read().split("\n")
    name = os.path.basename(path.split(".")[0])

    done = []
    for t in tiles:
        tile = os.path.join(common.target_dir_step4, t + ".MAP")
        if os.path.exists(tile):
            done.append(t)
            
    if verbose:     
        i = 0       
        for t in tiles:
            if not t in done:
                i += 1
                print(i, t, "not found")
        
            
    dst_name = name + "_map.zip"
    dst = os.path.join(common.target_dir_step5, dst_name)
    zip_exist = os.path.exists(dst)

    if len(done) == len(tiles) and not zip_exist:
        os.system("./pack.py '%s'" % path)
            
    dst_name = name + "_map.zip"
    dst = os.path.join(common.target_dir_step5, dst_name)
    zip_exist = os.path.exists(dst)

    dst_name = name + "_agl.zip"
    dst = os.path.join(common.target_dir_step5, dst_name)
    agl_exist = os.path.exists(dst)            
            
            
    line = (name, len(done), len(tiles), zip_exist, agl_exist)
    lines.append(line)

lines.sort(key = lambda a: a[1] * 100000000 / a[2] + a[2], reverse=True)    

for line in lines:
    name, done, cnt, zip_exist, agl_exist = line
    per = (done / cnt) * 100
    tiles = "%u/%u" % (done, cnt)
    z = "MAP" if zip_exist else ""
    a = "AGL" if agl_exist else ""
    print("%60s\t%3u%% %10s %5s%5s" % (name, per, tiles, z, a))

