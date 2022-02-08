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

    tiles = {}

    for filename in files:
        source = os.path.join(source_dir, filename)
        target_filename = filename.split(".")[0] + ".list"
        target = os.path.join(target_dir, target_filename)

        print("Opening file: %s" % source)
        data = json.loads(open(source, "r").read())

        if data["features"][0]["geometry"]["type"] == "Polygon":
            coords = [data["features"][0]["geometry"]["coordinates"]]
        else:
            coords = data["features"][0]["geometry"]["coordinates"]


        for a in coords:
            for c in a:
                poly = Polygon(c)
                lon1, lat1, lon2, lat2 = map(int, poly.bounds)
                
                if lon1 < 0:
                    lon1 -=1
                if lat1 < 0:
                    lat1 -=1
                if lon2 > 0:
                    lon2 +=1
                if lat2 > 0:
                    lat2 +=1

                print("Bounds", lon1, lat1, lon2, lat2)
                print("lon", list(range(lon1, lon2)))
                print("lat", list(range(lat1, lat2)))

                for lon in range(lon1, lon2):
                    for lat in range(lat1, lat2):
                        print(lon, lat, common.tile_filename(lon, lat))
                        blat1 = lat
                        blon1 = lon
                        blat2 = lat + 1
                        blon2 = lon + 1
                        
                        bbox = Polygon(([blon1, blat1], [blon1, blat2], [blon2, blat2], [blon2, blat1]))                        
#                        print("intersect", bbox.intersects(poly))
                        if bbox.intersects(poly):                        
                            tile = common.tile_filename(lon, lat)
                            if (tile not in tiles): 
                                valid = (tile in valid_tiles)
                                tiles[tile] = (blon1, blat1, blon2, blat2, [filename.split(".")[0]], valid)
                            else:                                
                                tiles[tile][4].append(filename.split(".")[0])
                     
    features = []    
    for tile in tiles:
        blon1, blat1, blon2, blat2, countries, valid = tiles[tile]
        feature = {}
        feature["type"] = "Feature"
        feature["geometry"] = {"type": "Polygon", "coordinates": [[[blon1, blat1], [blon1, blat2], [blon2, blat2], [blon2, blat1]]]}
        feature["properties"] = {"name": tile, "valid" : valid}
        for c in countries:
            feature["properties"][c] = True
        features.append(feature)
        

    data = {}
    data["type"] = "FeatureCollection"
    data["features"] = features
    open("verify_tiles.geojson", "w").write(json.dumps(data))
        
if __name__ == '__main__':
    if len(sys.argv) > 1:  
        country = sys.argv[1]
    else:
        country = None
        
    pipeline_get_list(country)

def test(lon, lat):
    print(lon, lat, common.tile_filename(lon, lat))
    
    
test(0,0)    
test(1,1)    
test(1,-1)    
test(-1,1)    
test(-1,-1)    
test(-1,0)    
test(1,0)    
test(0,-1)    
test(0,1)    








