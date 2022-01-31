#!/usr/bin/python3
#this script is for getting osm data for tile

import os
import common

def pipeline_step1():
    params = {
        "lon1": common.lon,
        "lat1": common.lat,
        "lon2": common.lon + common.step,
        "lat2": common.lat + common.step,
    }

    tile_name = common.tile_name
    target_dir = common.target_dir_step1

    os.makedirs(target_dir, exist_ok = True)

    print("Geting data for tile %s" % tile_name)
    print("  source OSM")
    print("  path %s" % target_dir)

    #list scripts
    scripts = os.listdir(common.osm_script_dir)

    for script in scripts:
        if script[-1] == "~":
            continue
        
        filename = tile_name + "_" + script + ".osmjson"
        full_path = os.path.join(target_dir, filename)

        if os.path.exists(full_path):
            print("Skipping script %s, %s exists" % (script, filename))
            continue

        print("Executing script %s" % script)

        #load script
        data = open(os.path.join(common.osm_script_dir, script), "r").read()

        #assign parameters
        for k in params:
            data = data.replace("%%%s%%" % k, str(params[k]))

        #get result
        result = common.query_overpass(data)
        
        #save results
        f = open(full_path, "wb")
        f.write(result)
        f.close()
        
        #invalidate next steps
        common.invalidate_step(2, script)
        
    print("Done")
    
if __name__ == '__main__':  
    common.init_vars()
    pipeline_step1()
    
