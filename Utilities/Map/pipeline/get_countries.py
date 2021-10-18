#!/usr/bin/python3
#this script is for getting countri list

import os
import common

def pipeline_countries():
    target_file = common.target_countries

    print("Geting countries list")
    print("  source OSM")
    print("  path %s" % target_file)

    script = "countries.overpass"
    
    print("Executing script %s" % script)

    #load script
    data = open(os.path.join(common.assets_dir, script), "r").read()

    #get result
    result = common.query_overpass(data)
        
    #save results
    f = open(target_file, "wb")
    f.write(result)
    f.close()
        
    print("Done")
    
if __name__ == '__main__':  
    pipeline_countries()
    
