import json
import numpy as np
from urllib.request import urlopen
import sys
import os

def drop_geometry(source, target, drop):
    data = json.loads(open(source, "r").read())
    to_delete = []
    
    for feature_index in range(len(data["features"])):
        feature = data["features"][feature_index]
        geometry_type = feature["geometry"]["type"]
        
        if geometry_type in drop:
            to_delete.append(feature_index)
        
    if len(to_delete) == 0:
        return 0
            
    to_delete.reverse()
    for feature_index in to_delete:
        del data["features"][feature_index]
        
    f = open(target, "w")
    f.write(json.dumps(data))
    f.close()
    
    return len(to_delete)

def get_lonlat():
    try:
        lon = int(sys.argv[1])
        lat = int(sys.argv[2])
    except:
        print("Usage: ./%s [lon] [lat]" % os.path.basename(sys.argv[0]))
        print("   eg: ./%s 17 48" % os.path.basename(sys.argv[0]))
    
        sys.exit(-1)
    return lon, lat

def query_overpass(query):
    query = query.encode("utf-8")

    try:
        f = urlopen(overpass_url, query)
    except HTTPError as e:
        f = e
        
    response = f.read(overpass_read_chunk_size)
    while True:
        data = f.read(overpass_read_chunk_size)
        if len(data) == 0:
            break
        response = response + data
    f.close()
    
    if f.code == 200:
        return response
        
    if f.code == 400:
        raise Exception("Bad request")
    if f.code == 429:
        raise Exception("Too Many Requests")
    if f.code == 504:
        raise Exception("Gateway Timeout")
    raise Exception("Unknown error code %u" % f.code)        

def tile_filename(lon, lat):
    lat_n = abs(int(lat))
    lon_n = abs(int(lon))

    if lat >= 0:
        lat_c = "N"
    else:
        lat_c = "S"
        lat_n -= 1

    if lon >= 0:
        lon_c = "E"
    else:
        lon_c = "W"
        lon_n -= 1
        
    return "%c%02u%c%03u" % (lat_c, abs(lat_n), lon_c, abs(lon_n))
    

def create_grid(fname, lon1, lat1, lon2, lat2, lon_step, lat_step):
    body = {}
    body["type"] = "GeometryCollection"
    body["geometries"] = []
    
    for x in np.arange(lon1, lon2, lon_step):
        for y in np.arange(lat1, lat2, lat_step):
            poly = {}
            poly["type"] = "Polygon"
            poly["coordinates"] = []
            poly["coordinates"].append([x, y])        
            poly["coordinates"].append([x, y + lat_step])
            poly["coordinates"].append([x + lon_step, y + lat_step])        
            poly["coordinates"].append([x + lon_step, y])
            poly["coordinates"] = [poly["coordinates"]]
            body["geometries"].append(poly)
    
    f = open(fname, "w")
    f.write(json.dumps(body))
    f.close()
    
def invalidate_step(step, layer = None):
    if step == 2:
        path = os.path.join(target_dir_step2, tile_name + "_" + layer + ".geojson")
        if os.path.exists(path):
            print("Invalidationg step %u, removing file %s" % (step, path))
            os.remove(path)
        invalidate_step(3, layer)
    
    if step == 3:
        path = os.path.join(target_dir_step3, tile_name + "_" + layer + ".geojson")
        if os.path.exists(path):
            print("Invalidationg step %u, removing file %s" % (step, path))
            os.remove(path)
        invalidate_step(4)

    if step == 4:
        path = os.path.join(target_dir_step4, tile_name + ".geojson")
        if os.path.exists(path):
            print("Invalidationg step %u, removing file %s" % (step, path))
            os.remove(path)

    
    
# VARIABILES SHARED BETWEEN STEPS

#paths
storage_path = "/media/horinek/topo_data/OSM/data/"

lon, lat = get_lonlat()
tile_name = tile_filename(lon, lat)
target_dir_step1 = os.path.join(storage_path, "step1", tile_name)
target_dir_step2 = os.path.join(storage_path, "step2", tile_name)
target_dir_step3 = os.path.join(storage_path, "step3", tile_name)
target_dir_step4 = os.path.join(storage_path, "step4")


osm_script_dir = "osm_scripts"
mapshaper_scripts_dir = "mapshaper_scripts"

local_dir = os.path.dirname(os.path.realpath(__file__))
assets_dir = os.path.join(local_dir, "assets")

#constants
step = 1
split = 8
GPS_COORD_MUL = 10000000

overpass_read_chunk_size = 4096
overpass_url = "http://overpass-api.de/api/interpreter"
