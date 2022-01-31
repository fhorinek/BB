#!/usr/bin/python3
#this script is for data pre-processing (simplify, slice, crop, remove attributes)

import os
import common

def pipeline_step3():
    source_dir = common.target_dir_step2
    target_dir = common.target_dir_step3
    os.makedirs(target_dir, exist_ok = True)

    params = {
        "lon1": common.lon,
        "lat1": common.lat,
        "lon2": common.lon + common.step,
        "lat2": common.lat + common.step,
        "split": common.split,
        "assets": common.assets_dir,
        "tile_name": common.tile_filename
    }

    print("Processing tile %s" % common.tile_name)
    print("  source %s" % source_dir)
    print("  target %s" % target_dir)

    #generate grid
    grid = os.path.join(common.target_dir_step3, "grid.geojson")
    if not os.path.exists(grid):
        step = 1.0 / common.split
        common.create_grid(grid, common.lon, common.lat, common.lon + common.step, common.lat + common.step, step, step)

    #list files
    files = os.listdir(source_dir)
    files.append("borders")

    for filename in files:
        if filename == "borders":
            source = os.path.join(common.assets_dir, "borders.json")
            layer = "borders"
            filename = "%s_borders.geojson" % common.tile_filename(common.lon, common.lat)
        else:
            source = os.path.join(source_dir, filename)
            layer = os.path.splitext(filename)[0].split("_")[1]
            
        target = os.path.join(target_dir, filename)
            
        script_path = os.path.join(common.mapshaper_scripts_dir, layer)


        if not os.path.exists(script_path):
            raise Exception("Procesing script for %s does not exists" % (filename))
        
        if os.path.exists(target):
            print("Skipping, target file %s exists" % (filename))
            continue

            
        if common.geojson_empty(source):
            print("Skipping, source file %s empty" % (filename))
            continue

        print("Processing file %s" % filename)


        #load script
        script = open(script_path, "r").read()

        #assign parameters
        params["name"] = os.path.splitext(filename)[0]   
        for k in params:
            script = script.replace("%%%s%%" % k, str(params[k]))   
        
        use_grid = script.find("#USE_GRID") >= 0
        default_output = script.find("#CUSTOM_OUTPUT") == -1
        
        drop = []
        add = []
        if script.find("#DROP_POLYGONS") >= 0:
            drop.append("Polygon")
        if script.find("#DROP_LINES") >= 0:
            drop.append("LineString")
            drop.append("MultiLineString")
        if script.find("#DROP_POINTS") >= 0:
            drop.append("Point")
        if script.find("#INJECT_LINES") >= 0:
            add.append("LineString")
        if script.find("#INJECT_POLYGONS") >= 0:
            add.append("Polygon")
        if script.find("#INJECT_POINT") >= 0:
            add.append("Point")
        
        tmp_file = None
        if len(drop) > 0 or len(add) > 0:
            tmp_file = "%s_mapshaper_temporary_input.geojson" % common.tile_name
            dropped, added = common.mod_geometry(source, tmp_file, drop, add)
            if dropped > 0 or added > 0:
                if dropped > 0:
                    print("Dropping %u features %s" % (dropped, str(drop)))
                if added > 0:
                    print("Adding %u features %s" % (added, str(add)))
                source = tmp_file
            else:
                tmp_file = None

        
        cmd = "mapshaper-xl -i %s \\\n" % source
        
        if use_grid:
            cmd += "   -i %s \\\n" % grid
            
        for line in script.split("\n"):
            #skip empty line
            if len(line.split()) == 0:
                continue
            #skip comment
            if line[0] == "#":
                continue
            #add command to queue            
            cmd += "   -%s \\\n" % line


        if default_output:
            if use_grid:
                cmd += "   -drop target=grid \\\n"
        
            cmd += "   -o %s format=geojson combine-layers" % target
        
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
    common.init_vars()
    pipeline_step3()
    
