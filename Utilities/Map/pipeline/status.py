#!/usr/bin/python

import sys
import os
import common


countries = os.listdir(common.target_dir_countries)

lines = []
for c in countries:
    path = os.path.join(common.target_dir_countries, c)
    tiles = open(path, "r").read().split("\n")
    name = os.path.basename(path.split(".")[0])

    done = 0
    for t in tiles:
        tile = os.path.join(common.target_dir_step4, t + ".MAP")
        if os.path.exists(tile):
            done += 1
            
    dst_name = name + ".zip"
    dst = os.path.join(common.target_dir_step5, dst_name)
    zip_exist = os.path.exists(dst)

    dst_name = name + "_agl.zip"
    dst = os.path.join(common.target_dir_step5, dst_name)
    agl_exist = os.path.exists(dst)
            
            
    line = (name, done, len(tiles), zip_exist, agl_exist)
    lines.append(line)


lines.sort(key = lambda a: a[1] * 1000000 / a[2] + a[2], reverse=True)    

for line in lines:
    name, done, cnt, zip_exist, agl_exist = line
    per = (done / cnt) * 100
    tiles = "%u/%u" % (done, cnt)
    z = "MAP" if zip_exist else ""
    a = "AGL" if agl_exist else ""
    print("%60s\t%3u%% %10s %5s%5s" % (name, per, tiles, z, a))

