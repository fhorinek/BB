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
    
for c in countries:
    path = os.path.join(common.target_dir_countries, c)
    tiles = open(path, "r").read().split("\n")
    name = os.path.basename(path.split(".")[0])

    done = []
    for t in tiles:
        tile = os.path.join(common.target_dir_step4, t + ".MAP")
        if os.path.exists(tile):
            done.append(t)
            
    for t in tiles:
        if not t in done:
            ret = os.system("./update.py %s" % t)
            if ret != 0:
                sys.exit(ret)


