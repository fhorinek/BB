#!/usr/bin/python

import common
import os
import sys

remove = ""

if len(sys.argv) == 4:
    remove = sys.argv[1]
    step = sys.argv[2]
    layer = sys.argv[3]

if remove != "remove":
    print("Usage: %s remove [step || all] [layer || all]" % (sys.argv[0]))
    sys.exit(-1)

if step == "all":    
    steps = ["step1", "step2", "step3", "step4"]
else:
    steps = step.split(",")

layers = layer.split(",")

removed = 0
for step in steps:
    step_path = os.path.join(common.storage_path, step)
    for tile in os.listdir(step_path):
        tile_path = os.path.join(step_path, tile)
        for layer in os.listdir(tile_path):
            ln = layer.replace("%s_" % tile, "").split(".")[0]
            if ln in layers or layer == "all":
                file_path = os.path.join(tile_path, layer)
                removed += 1
                os.remove(file_path)
#                print("%u removing %s" % (removed, file_path))
                
print("Removed %u files" % removed)
                




