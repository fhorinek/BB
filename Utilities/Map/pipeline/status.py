#!/usr/bin/python

import sys
import os
import common


countries = os.listdir(common.target_dir_countries)

lines = []
for c in countries:
    path = os.path.join(common.target_dir_countries, c)
    tiles = open(path, "r").read().split("\n")

    done = 0
    for t in tiles:
        tile = os.path.join(common.target_dir_step4, t + ".MAP")
        if os.path.exists(tile):
            done += 1
            
            
    line = (c, done, len(tiles))
    lines.append(line)


lines.sort(key = lambda a: a[1] * 1000000 / a[2] + a[2], reverse=True)    

for line in lines:
    name, done, cnt = line
    per = (done / cnt) * 100
    print("%60s\t%3u%%\t%u/%u  " % (name, per, done, cnt))

