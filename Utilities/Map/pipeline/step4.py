#!/usr/bin/python3
#
# This script converts pre-processed data to binary format for BB

import json
import struct
import os
from shapely.geometry import Polygon, MultiPolygon, Point, LineString

from datetime import datetime 
from collections import OrderedDict
from pprint import pp

import common

from MAP_TYPES import *

#print file structure
debug_bin = False
#create geojson files contaning duplicates
debug_duplicates = False
#print features info 
debug_features = False

# Create 1 or more features from the given dict_data. Normally the
# dict_data contains 1 feature. But sometimes we derive more than 1
# feature and therefore return a list of Feature objects.
#
# @param dict_data a JSON describing a feature to read
#
# @return list of Feature objects derived from dict_data
#
def createFeatures(dict_data):

    # This is the result list, containing all Feature objects that we return
    features = []

    # This is the main Feature, where we work on
    feature = Feature()
    features.append(feature)

    feature.source_data = dict_data

    #feature was sliced with grid, it will not overlap the boundary
    feature.sliced = "sliced" in dict_data["properties"]

    if dict_data["geometry"]["type"] == "Point":
        feature.elevation = -32768
        feature.name = ""
        lon, lat = dict_data["geometry"]['coordinates']

        if "name" in dict_data["properties"]:
            feature.name = dict_data["properties"]["name"]
        if "ele" in dict_data["properties"]:
            ele = dict_data["properties"]["ele"].split(";")[0].replace(",", ".").replace("~","").replace("m","")
            try:
                feature.elevation = int(float(ele))
                assert feature.elevation < 9000
            except (ValueError, AssertionError):
                print("Wrong elevation value")
                print(dict_data)
                feature.valid = False
                return None

        if dict_data["properties"]["type"] == "peak":
            feature.type = MAP_TYPE_POI_PEAK

        if dict_data["properties"]["type"] == "city":
            feature.type = MAP_TYPE_POI_CITY
        if dict_data["properties"]["type"] == "town":
            feature.type = MAP_TYPE_POI_TOWN
        if dict_data["properties"]["type"] == "village":
            feature.type = MAP_TYPE_POI_VILLAGE
        if dict_data["properties"]["type"] in ["hamlet", "isolated_dwelling"]:
            feature.type = MAP_TYPE_POI_HAMLET

        if feature.type != -1:
            feature.savePOI(feature.type, feature.name, feature.elevation, lon, lat)
            feature.geometry = Point(lon, lat)

    if dict_data["geometry"]["type"] == "LineString":
        if dict_data["properties"]["type"] == "border":
           feature.type = MAP_TYPE_LINE_BORDER

        if dict_data["properties"]["type"] == "river":
            feature.type = MAP_TYPE_LINE_RIVER

        if dict_data["properties"]["type"] == "highway":
            feature.type = MAP_TYPE_LINE_HIGHWAY
        if dict_data["properties"]["type"] == "primary":
            feature.type = MAP_TYPE_LINE_PRIMARY
        if dict_data["properties"]["type"] == "secondary":
            feature.type = MAP_TYPE_LINE_SECONDARY
        if dict_data["properties"]["type"] == "tertiary":
            feature.type = MAP_TYPE_LINE_TERTIARY

        if dict_data["properties"]["type"] == "rail":
            feature.type = MAP_TYPE_LINE_RAIL

        if dict_data["properties"]["type"] == "power":
           feature.type = MAP_TYPE_LINE_POWER

        if dict_data["properties"]["type"] == "aerialway":
           feature.type = MAP_TYPE_LINE_AERIALWAY
           
        if dict_data["properties"]["type"] == "aerialway_low":
            feature.type = MAP_TYPE_LINE_AERIALWAY_LOW


        number_of_points = len(dict_data["geometry"]['coordinates'])
        assert(number_of_points > 0)
        assert(number_of_points <= 0xFFFF)

        if feature.type != -1:
            #type
            feature.data += struct.pack("B", feature.type)
            #pad
            feature.data += struct.pack("B", 0)
            #number of points
            feature.data += struct.pack("<H", number_of_points)
            #data
            for pos in dict_data["geometry"]['coordinates']:
                lon, lat = pos
                feature.data += struct.pack("<l", int(lon * common.GPS_COORD_MUL))
                feature.data += struct.pack("<l", int(lat * common.GPS_COORD_MUL))

            feature.geometry = LineString(dict_data["geometry"]['coordinates'])


    if dict_data["geometry"]["type"] == "Polygon":
        if "type" in dict_data["properties"]:
            poi_type = None
            if dict_data["properties"]["type"] == "water":
                feature.type = MAP_TYPE_POLYGON_WATER

            if dict_data["properties"]["type"] == "resident":
                feature.type = MAP_TYPE_POLYGON_RESIDENT

            if dict_data["properties"]["type"] == "aeroway":
                feature.type = MAP_TYPE_POLYGON_AEROWAY
                poi_type = MAP_TYPE_POI_AEROWAY

            if dict_data["properties"]["type"] == "free_flying":
                if dict_data["properties"]["free_flying:site"] == "takeoff":
                    feature.type = MAP_TYPE_POLYGON_TAKEOFF
                    poi_type = MAP_TYPE_POI_TAKEOFF
                elif dict_data["properties"]["free_flying:site"] == "landing":
                    feature.type = MAP_TYPE_POLYGON_LANDING
                    poi_type = MAP_TYPE_POI_LANDING
                else:
                    print(dict_data)
                    raise Exception("No meaningfull free_flying:site found")

            if poi_type != None:
                # We add an additional POI at innerX/Y of the polygon
                lon = dict_data["properties"]["innerX"]
                lat = dict_data["properties"]["innerY"]
                name = ""
                f = Feature()
                f.sliced = feature.sliced
                f.source_data = dict_data
                f.dict_data = dict_data
                f.type = poi_type
                f.name = name
                f.savePOI(poi_type, name, 0, lon, lat)
                f.geometry = Point(lon, lat)
                features.append(f)

        points = []

        i = 1
        for part in dict_data["geometry"]['coordinates']:
            points += part
            if i < len(dict_data["geometry"]['coordinates']):
                points += [[0x7FFFFFFF / common.GPS_COORD_MUL, 0x7FFFFFFF / common.GPS_COORD_MUL]]
            i += 1

        number_of_points = len(points)

        if (number_of_points == 0 or number_of_points > 0xFFFF):
            print(dict_data)
            tmp = Polygon(points).bounds
            print("rectangle name=error bbox=%f,%f,%f,%f" % tmp)
            assert(0)

        if feature.type != -1:
            #type
            feature.data += struct.pack("B", feature.type)
            #pad
            feature.data += struct.pack("B", 0)
            #number of points
            feature.data += struct.pack("<H", number_of_points)
            #data
            for pos in points:
                lon, lat = pos
                feature.data += struct.pack("<l", int(lon * common.GPS_COORD_MUL))
                feature.data += struct.pack("<l", int(lat * common.GPS_COORD_MUL))


            polygons = []
            for coords in dict_data["geometry"]['coordinates']:
                polygons.append(Polygon(coords))
            feature.geometry = MultiPolygon(polygons)

    if feature.type == -1:
        print(dict_data)
        raise Exception("Unable to parse feature")

    if debug_features:
        feature.dict_data = dict_data

    return features

def feature_factory(dict_data):
    output = []
    
    if dict_data["geometry"] == None:
        return output
    
    if dict_data["geometry"]["type"] == "MultiLineString":
        print("Converting MultiLineString into %u LineStrings" % len(dict_data["geometry"]['coordinates']))
        base = dict(dict_data)
        base["geometry"]["type"] = "LineString"
        
        for part in dict_data["geometry"]['coordinates']:
            base["geometry"]['coordinates'] = part
            features = createFeatures(base)
            for f in features:
                if f.valid:
                    output.append(f)
                
    elif dict_data["geometry"]["type"] == "MultiPolygon":
        print("Converting MultiPolygon into %u Polygons" % len(dict_data["geometry"]['coordinates']))
        base = dict(dict_data)
        base["geometry"]["type"] = "Polygon"
        
        for part in dict_data["geometry"]['coordinates']:
            base["geometry"]['coordinates'] = part
            features = createFeatures(base)
            for f in features:
                if f.valid:
                    output.append(f)
    else:
        features = createFeatures(dict_data)
        for f in features:
            if f.valid:
                output.append(f)
        
    return output
    


class Feature(object):
    def __init__(self):
        self.data = bytes()
        self.type = -1
        self.valid = True

    def savePOI(self, type, name, elevation, lon, lat):
        name_lenght = len(name.encode("UTF-8"))
        assert(name_lenght <= 0xFF)
            
        #type
        self.data += struct.pack("B", type)
        #name lenght
        self.data += struct.pack("B", name_lenght)
        #elevation
        self.data += struct.pack("<h", elevation)
        #data
        self.data += struct.pack("<l", int(lon * common.GPS_COORD_MUL))
        self.data += struct.pack("<l", int(lat * common.GPS_COORD_MUL))
        self.data += name.encode("UTF-8")
        while len(self.data) % 4 != 0:
            self.data += struct.pack("B", 0)

    def inside(self, bbox):
        if self.sliced:
            return bbox.contains(self.geometry)
        else:
            return bbox.intersects(self.geometry)

    def get_data(self, addr):
        assert(addr % 4 == 0)
        self.addr = addr
        return self.data
        
    def get_addr(self):
        return self.addr
        
    def print_debug(self, start_address):
        print("[%08X]" % (self.addr + start_address))            
        print(self.dict_data)
        tmp = []
        for c in self.data:
            tmp.append("%02X" % c)
        print(" * " + " ".join(tmp))
        print()

       
def pipeline_step4():    
    source_dir = common.target_dir_step3
    target_dir = common.target_dir_step4
    os.makedirs(target_dir, exist_ok = True)

    target = os.path.join(target_dir, common.tile_name + ".MAP")

    print("Converting layers for tile %s" % common.tile_name)
    print("  source %s" % source_dir)
    print("  target %s" % target)
    
    if os.path.exists(target):
        print("Map file %s exist, skipping" % common.tile_name)
        return

    w_cnt = common.split
    h_cnt = common.split
    w_delta = 1.0 / w_cnt
    h_delta = 1.0 / h_cnt

    grid = []
    features = []
    features_data = bytes()

    for i in range(w_cnt * h_cnt):
        grid.append([])

    print("Grid is %u x %u" % (w_cnt, h_cnt))

    #list files
    files = open(os.path.join(common.config_dir, "layer_order"), "r").read().split("\n")
    for filename in files:
        if len(filename) == 0:
            continue
            
        source = os.path.join(source_dir, "%s_%s.geojson" % (common.tile_name, filename))
        
        if not os.path.exists(source):
            raise Exception("Source data %s not found" % (filename))
        
        print("Opening file: %s" % source, end="")
        if os.path.getsize(source) > 0:
            data = json.loads(open(source, "r").read())
            print()
        else:
            data = []
            print(" (Empty)")

        if "features" in data:
            for dict_data in data["features"]:
                features.extend(feature_factory(dict_data))

    for f in features:
        features_data += f.get_data(len(features_data))

    print("Loaded %u features" % len(features))

    print("Indexing...", end="", flush = True)

    for y in range(h_cnt):
        for x in range(w_cnt):
            lat1 = common.lat + h_delta * y
            lon1 = common.lon + w_delta * x
            lat2 = lat1 + h_delta
            lon2 = lon1 + w_delta
            
            bbox = Polygon(([lon1, lat1], [lon1, lat2], [lon2, lat2], [lon2, lat1]))
            
            grid_index = y * w_cnt + x
            
            for feature_index in range(len(features)):
                f = features[feature_index]

                if f.inside(bbox):
                    grid[grid_index].append(feature_index)


    print("done")

    print("Checking for missing features...", end="", flush = True)

    index_records = 0
    index_contains = []
    duplicates = []
    duplicates_cnt = 0
    for g in grid:
        index_records += len(g)     
        for i in g:   
            if i not in index_contains:
                index_contains.append(i)
            else:
                duplicates_cnt += 1
                if i not in duplicates:            
                    duplicates.append(i)
       
    missing = []
    for feature_index in range(len(features)):
        if feature_index not in index_contains:
            missing.append(features[feature_index])

    if len(missing) > 0:            
        print("Error: Missing %u features in grid index" % len(missing))
        missing_file = {}
        missing_file["type"] = "FeatureCollection"
        missing_file["features"] = []
        for m in missing:
            missing_file["features"].append(m.source_data)
            
        path = os.path.join(target_dir, common.tile_name + "_missing.geojson")            
        f = open(path, "w")
        f.write(json.dumps(missing_file))
        f.close()
            
        assert(0)
        
    if duplicates_cnt > 0 and debug_duplicates:            
        duplicate_file = {}
        duplicate_file["type"] = "FeatureCollection"
        duplicate_file["features"] = []
        for di in duplicates:
            d = features[di]
            duplicate_file["features"].append(d.source_data)
            
        path = os.path.join(target_dir, common.tile_name + "_duplicate.geojson")            
        f = open(path, "w")
        f.write(json.dumps(duplicate_file))
        f.close()

    print("done")

    print("Index contains %u records (%u duplicates)" % (index_records,  duplicates_cnt))

    file_data = bytes()

    if debug_bin:
        print("\n%08X *** writing header" % len(file_data))
    #header = 0
    # 8b id
    # 8b version
    # 8b grid w
    # 8b grid h
    magic = (2 % 127) + 1
    file_data += struct.pack("<BBBB", 0x55, magic, w_cnt, h_cnt)

    # 32b longitude
    file_data += struct.pack("<i", common.lon * common.GPS_COORD_MUL)
    
    # 32b latitude
    file_data += struct.pack("<i", common.lat * common.GPS_COORD_MUL)
            
        
    if debug_bin:
        print("\n%08X *** writing grid info" % len(file_data))
        print("         X x Y: [index address], [number of features]")
    # 32b start address
    # 32b number of features
    #           header size + size of grid
    index_start_address = 12 + (8 * w_cnt * h_cnt)
    acc = 0;        
    for y in range(h_cnt):
        for x in range(w_cnt):
            grid_index = y * w_cnt + x
            addr = index_start_address + acc
            if debug_bin:
                print("%08X %u x %u: %X, %u" % (len(file_data), x, y, addr, len(grid[grid_index])))
            file_data += struct.pack("<II", addr, len(grid[grid_index]))
            acc += 4 * len(grid[grid_index])
            
            
    #index = 12 + 8 * w * h
    # 32b feature address  
    if debug_bin:
        print("\n%08X *** writing index" % len(file_data))
        print("         [feature index]: [feature address]")
    features_stat_address = index_start_address + index_records * 4
    for segment in grid:
        for g in segment:
            addr = features[g].get_addr() + features_stat_address
            if debug_bin:
                print("%08X %4u: %X" % (len(file_data), g, addr))
            file_data += struct.pack("<I", addr)
        
        
    #features
    if debug_bin:
        print("\n%08X *** writing features" % len(file_data))
        
    if debug_features:
        for f in features:
            f.print_debug(features_stat_address)        
        
    file_data += features_data
    
    if debug_bin:
        print("\n%08X *** EOF" % len(file_data))

    f = open(target, "wb")
    f.write(file_data)
    f.close()

    print("total size %0.2fkB (%u b)" % (len(file_data) / 1024.0, len(file_data)))
    
if __name__ == '__main__':  
    common.init_vars()
    pipeline_step4()    
