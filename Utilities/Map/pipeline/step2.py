#!/usr/bin/python3

import os
import common

def pipeline_step2():
    source_dir = common.target_dir_step1
    target_dir = common.target_dir_step2
    os.makedirs(target_dir, exist_ok = True)

    print("Converting data to geojson for tile %s" % common.tile_name)
    print("  source %s" % source_dir)
    print("  target %s" % target_dir)

    #list files
    files = os.listdir(source_dir)

    for filename in files:
        source = os.path.join(source_dir, filename)
        layer = os.path.splitext(filename)[0].split("_")[1]
        target_filename = os.path.splitext(filename)[0] + ".geojson"
        target = os.path.join(target_dir, target_filename)

        if os.path.exists(target):
            print("Skipping %s, %s exists" % (filename, target_filename))
            continue

        print("Converting %s to %s" % (filename, target_filename))
        os.system("osmtogeojson %s > %s" % (source, target))
     
        #invalidate next steps
        common.invalidate_step(3, layer) 
        
    print("Done")
    
if __name__ == '__main__':  
    common.init_vars()
    pipeline_step2()
    
