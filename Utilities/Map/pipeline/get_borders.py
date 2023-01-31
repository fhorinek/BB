#!/usr/bin/python3
#this script is for getting osm data for tile

import os
import common
import sys

def pipeline_get_borders_raw():
    target_dir = common.target_dir_borders_raw

    os.makedirs(target_dir, exist_ok = True)

    print("Geting borders from OSM")
    print("  source OSM")
    print("  path %s" % target_dir)

    #list
    countries = open(common.target_countries, "r").read().split("\n")

    for c in countries:
        c = c.split("\t")
        if len(c) != 2:
            continue
        if c[0] == "":
            continue
            
        c[1] = c[1].replace("\"","").replace(",","").replace("'","")
            
        
        filename = c[0] + "_" + c[1] + ".osmjson"
        full_path = os.path.join(target_dir, filename)

        if os.path.exists(full_path):
            print("Skipping %s exists" % (filename))
            continue

        script = "border.overpassql"
        print("Executing script %s for %s" % (script, filename))

        #load script
        data = open(os.path.join(common.assets_dir, script), "r").read()

        params = {
            "ISO_CODE": c[0]
        }

        #assign parameters
        for k in params:
            data = data.replace("%%%s%%" % k, str(params[k]))

        #get result
        result = common.query_overpass(data)
        
        #save results
        f = open(full_path, "wb")
        f.write(result)
        f.close()

    print("Done")
    

def pipeline_get_borders_geo():
    source_dir = common.target_dir_borders_raw
    target_dir = common.target_dir_borders_geo
    os.makedirs(target_dir, exist_ok = True)

    print("Converting data to geojson")
    print("  source %s" % source_dir)
    print("  target %s" % target_dir)

    #list files
    files = os.listdir(source_dir)

    for filename in files:
        source = os.path.join(source_dir, filename)
        layer = os.path.splitext(filename)[0].split("_")[1]
        target_filename = os.path.splitext(filename)[0] + ".geojson"
        target = os.path.join(target_dir, target_filename)

        if os.path.exists(target) and os.path.getsize(target) > 0 :
            print("Skipping %s, %s exists" % (filename, target_filename))
            continue

        print("Converting %s to %s" % (filename, target_filename))
        ws = os.system("osmtogeojson '%s' > '%s'" % (source, target))
        ret = os.waitstatus_to_exitcode(ws)
        if ret != 0:
            sys.exit(ret)
        
    print("Done")
    
def pipeline_join_borders():
    source_dir = common.target_dir_borders_geo
    target = common.target_dir_borders_join

    print("Merging shapefiles")
    print("  source %s" % source_dir)
    print("  target %s" % target)
    
    files = os.listdir(source_dir)

    cmd = "mapshaper-xl -verbose\\\n"

    for filename in files:
        path = os.path.join(source_dir, filename)
        cmd += "   -i '%s' \\\n" % path
    
    cmd += "   -merge-layers target=* force"
    cmd += "   -simplify 30%"
    cmd += "   -lines"
    cmd += "   -dissolve"
    
    cmd += "   -o %s format=geojson combine-layers" % target
    ws = os.system(cmd)
    ret = os.waitstatus_to_exitcode(ws)
    if ret != 0:
        print(" ------ mapshaper command ------ ")
        print(cmd)
        print(" ------------------------------- ")
        raise Exception("Command not processed!")
    
if __name__ == '__main__':  
    pipeline_get_borders_raw()
    pipeline_get_borders_geo()
    pipeline_join_borders()
    
