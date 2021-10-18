#!/usr/bin/python3
#this script is for getting osm data for tile

import os
import common

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
            
        c[1] = c[1].replace("\"","").replace(",","")
            
        
        filename = c[0] + "_" + c[1] + ".osmjson"
        full_path = os.path.join(target_dir, filename)

        if os.path.exists(full_path):
            print("Skipping %s exists" % (filename))
            continue

        script = "border.overpass"
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

        if os.path.exists(target):
            print("Skipping %s, %s exists" % (filename, target_filename))
            continue

        print("Converting %s to %s" % (filename, target_filename))
        os.system("osmtogeojson '%s' > '%s'" % (source, target))

        
    print("Done")
    
def pipeline_get_borders_opti():
    source_dir = common.target_dir_borders_geo
    target_dir = common.target_dir_borders_opti
    os.makedirs(target_dir, exist_ok = True)

    print("Processing borders")
    print("  source %s" % source_dir)
    print("  target %s" % target_dir)

    #list files
    files = os.listdir(source_dir)

    for filename in files:
        source = os.path.join(source_dir, filename)
        
        layer = "border"
        script_path = os.path.join(common.mapshaper_scripts_dir, layer)

        if not os.path.exists(script_path):
            raise Exception("Procesing script for %s does not exists" % (filename))
        
        target = os.path.join(target_dir, filename)

        if os.path.exists(target):
            print("Skipping, target file %s exists" % (filename))
            continue

        print("Processing file %s" % filename)

        #load script
        script = open(script_path, "r").read()

        drop = []
        if script.find("#DROP_POLYGONS") >= 0:
            drop.append("Polygon")
        if script.find("#DROP_LINES") >= 0:
            drop.append("LineString")
            drop.append("MultiLineString")
        if script.find("#DROP_POINTS") >= 0:
            drop.append("Point")
        
        tmp_file = None
        if len(drop) > 0:
            tmp_file = "%s_mapshaper_temporary_input.geojson" % filename
            dropped = common.drop_geometry(source, tmp_file, drop)
            if dropped > 0:
                print("Dropping %u features %s" % (dropped, str(drop)))
                source = tmp_file
            else:
                tmp_file = None
        
        cmd = "mapshaper -i '%s' \\\n" % source
            
        for line in script.split("\n"):
            #skip empty line
            if len(line.split()) == 0:
                continue
            #skip comment
            if line[0] == "#":
                continue
            #add command to queue            
            cmd += "   -%s \\\n" % line


        cmd += "   -o '%s' format=geojson combine-layers" % target
        
        ret = os.system(cmd)
        
        common.invalidate_step(4, layer) 

        if ret != 0:
            print(" ------ mapshaper command ------ ")
            print(cmd)
            print(" ------------------------------- ")
            raise Exception("Command not processed!")
        
        if tmp_file:
            os.remove(tmp_file)
        
    print("Done")    
    
if __name__ == '__main__':  
    pipeline_get_borders_raw()
    pipeline_get_borders_geo()
    pipeline_get_borders_opti()
    
