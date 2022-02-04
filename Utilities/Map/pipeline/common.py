import json
import numpy as np
from urllib.request import urlopen
import sys
import os
import urllib


def mod_geometry(source, target, drop, add):
    data = json.loads(open(source, "r").read())
    to_delete = []
    
    for feature_index in range(len(data["features"])):
        feature = data["features"][feature_index]
        geometry_type = feature["geometry"]["type"]
        
        if geometry_type in drop:
            to_delete.append(feature_index)
        
    if len(to_delete) != 0:
        to_delete.reverse()
        for feature_index in to_delete:
            del data["features"][feature_index]

    
    for a in add:
        if a == "LineString":
            fl = {}
            fl["type"] = "Feature"
            fl["properties"] = {"aerialway":"dummy"}
            fl["geometry"] = {"type": "LineString", "coordinates": [[0, 0], [0, 0.1]]}
            data["features"].append(fl)

        if a == "Polygon":
            fp = {}
            fp["type"] = "Feature"
            fp["geometry"] = {"type": "Polygon", "coordinates": [[[0, 0], [0.1, 0.1], [0, 0.1], [0, 0]]]}
            data["features"].append(fp)

        if a == "Point":
            fp = {}
            fp["type"] = "Feature"
            fp["properties"] = {"name":"dummy", "ele": "0"}
            fp["geometry"] = {"type": "Point", "coordinates": [0, 0]}
            data["features"].append(fp)
    
    if len(to_delete) > 0 or len(add) > 0: 
        f = open(target, "w")
        f.write(json.dumps(data))
        f.close()
    
    return len(to_delete), len(add)

def get_lonlat():
    try:
        lon = int(sys.argv[1])
        lat = int(sys.argv[2])
    except:
        print("Usage: ./%s [lon] [lat]" % os.path.basename(sys.argv[0]))
        print("   eg: ./%s 17 48" % os.path.basename(sys.argv[0]))
    
        sys.exit(-1)
    return lon, lat

def query_overpass(orig_query):
    query = orig_query.encode("utf-8")

    try:
        f = urlopen(overpass_url, query)
    except urllib.error.HTTPError as e:
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
        
    print("---Failed query-------------")
    print(orig_query)
    print("----------------------------")
        
    if f.code == 400:
        raise Exception("Bad request")
    if f.code == 429:
        raise Exception("Too Many Requests")
    if f.code == 504:
        raise Exception("Gateway Timeout")
    raise Exception("Unknown error code %u" % f.code)        

def filename_to_lon_lat(name):
    lat_c = name[0]
    lat_n = int(name[1:3])
    lon_c = name[3]
    lon_n = int(name[4:7])
    
    if lat_c == "S":
        lat = -lat_n + 1
    else:
        lat = lat_n
       

    if lon_c == "W":
        lon = -lon_n + 1
    else:
        lon = lon_n
    
    return lon, lat


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
    
def geojson_empty(path):
    data = json.loads(open(path, "r").read())
    
    if data["type"] == "GeometryCollection":
        return len(data["geometries"]) == 0        
    else:
        return len(data["features"]) == 0

    
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
        path = os.path.join(target_dir_step4, tile_name + ".MAP")
        if os.path.exists(path):
            print("Invalidationg step %u, removing file %s" % (step, path))
            os.remove(path)

    
    
# VARIABILES SHARED BETWEEN STEPS

#paths
storage_path = "/media/horinek/topo_data/OSM/data/"
hgt_path = "/media/horinek/topo_data/HGT3/data/hgt/"

target_countries = os.path.join(storage_path, "countries.list")
target_dir_borders_raw = os.path.join(storage_path, "borders", "raw")
target_dir_borders_geo = os.path.join(storage_path, "borders", "geo")
target_dir_borders_opti = os.path.join(storage_path, "borders", "opti")
target_dir_countries = os.path.join(storage_path, "countries")

target_dir_step4 = os.path.join(storage_path, "step4")
target_dir_step5 = os.path.join(storage_path, "../dist")

def init_vars(lon_i = None, lat_i = None):
    global lon
    global lat
    global tile_name
    global target_dir_step1
    global target_dir_step2
    global target_dir_step3
    global target_dir_step4
    
    if (lon_i == None and lat_i == None):
        lon, lat = get_lonlat()
    else:
        lon = lon_i
        lat = lat_i
    tile_name = tile_filename(lon, lat)
    target_dir_step1 = os.path.join(storage_path, "step1", tile_name)
    target_dir_step2 = os.path.join(storage_path, "step2", tile_name)
    target_dir_step3 = os.path.join(storage_path, "step3", tile_name)


config_dir = "config"
osm_script_dir = "osm_scripts"
mapshaper_scripts_dir = "mapshaper_scripts"

local_dir = os.path.dirname(os.path.realpath(__file__))
assets_dir = os.path.join(local_dir, "assets")

#constants
step = 1
split = 16
GPS_COORD_MUL = 10000000

overpass_read_chunk_size = 4096
#overpass_url = "http://overpass-api.de/api/interpreter"
#overpass_url = "https://lz4.overpass-api.de/api/interpreter"
#overpass_url = "https://overpass.kumi.systems/api/interpreter"
overpass_url = "http://192.168.200.2:12346/api/interpreter"

