#!/usr/bin/python3

import os
import common
import json
import sys
from shapely.geometry import Polygon

def pipeline_get_list(country = None):
    valid_tiles = common.get_valid_tiles()

    source_dir = common.target_dir_borders_geo
    target_dir = common.target_dir_countries
    os.makedirs(target_dir, exist_ok = True)

    print("Assign tile to countrie")
    print("  source %s" % source_dir)
    print("  target %s" % target_dir)

    #list files
    if country == None:
        files = os.listdir(source_dir)
    else:
        files = [country + ".geojson"]

    for filename in files:
        source = os.path.join(source_dir, filename)
        target_filename = filename.split(".")[0] + ".list"
        target = os.path.join(target_dir, target_filename)

        if os.path.exists(target):
            print("Skipping %s, %s exists" % (filename, target_filename))
            continue

        print("Converting %s to %s" % (filename, target_filename))

        print("Opening file: %s" % source)
        data = json.loads(open(source, "r").read())

        if data["features"][0]["geometry"]["type"] == "Polygon":
            coords = [data["features"][0]["geometry"]["coordinates"]]
        else:
            coords = data["features"][0]["geometry"]["coordinates"]

        tiles = []

        for a in coords:
            for c in a:
                poly = Polygon(c)
                lon1, lat1, lon2, lat2 = map(int, poly.bounds)
                
                if lon1 < 0:
                    lon1 = max(-180, lon1 - 1)
                if lat1 < 0:
                    lat1 = max(-90, lat1 - 1)
                if lon2 > 0:
                    lon2 = min(180, lon2 + 1)
                if lat2 > 0:
                    lat2 = min(90, lat2 + 1)


#                print("Bounds", lon1, lat1, lon2, lat2)
#                print("lon", list(range(lon1, lon2)))
#                print("lat", list(range(lat1, lat2)))

                for lon in range(lon1, lon2):
                    for lat in range(lat1, lat2):
                        blat1 = lat
                        blon1 = lon
                        blat2 = lat + 1
                        blon2 = lon + 1
                        
                        bbox = Polygon(([blon1, blat1], [blon1, blat2], [blon2, blat2], [blon2, blat1]))                        
#                        print("intersect", bbox.intersects(poly))
                        if bbox.intersects(poly):                        
                            tile = common.tile_filename(lon, lat)
 #                           print("-rectangle bbox=%d,%d,%d,%d \\" %(blon1, blat1, blon2, blat2))
                            if tile not in tiles:
                                tiles.append(tile)
        
 #       text = "mapshaper -i '%s' \\\n" % source
        text = "\n".join(tiles)
 #       text += "\n -o '%s_test.geojson' format=geojson combine-layers" % (target.split(".")[0])
        if len(tiles) > 0:
            open(target, "w").write(text)
        else:
            print("No valid tiles for %s" % filename)
        
#        os.system("bash '%s'" % target)



        
    print("Done")
    
   
    
if __name__ == '__main__':
    if len(sys.argv) > 1:  
        country = sys.argv[1]
    else:
        country = None
        
    pipeline_get_list(country)

    
