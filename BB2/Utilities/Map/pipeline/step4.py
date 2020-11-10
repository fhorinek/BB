#!/usr/bin/python3

import json
import struct
import os
from shapely.geometry import Polygon, Point, LineString

from datetime import datetime 
from collections import OrderedDict

import common

def feature_factory(dict_data):
    output = []
    
    if dict_data["geometry"]["type"] == "MultiLineString":
        print("Converting MultiLineString into %u LineStrings" % len(dict_data["geometry"]['coordinates']))
        base = dict(dict_data)
        base["geometry"]["type"] = "LineString"
        
        for part in dict_data["geometry"]['coordinates']:
            base["geometry"]['coordinates'] = part
            output.append(Feature(base))
    else:
        output.append(Feature(dict_data))
        
    return output
    

class Feature(object):
    def __init__(self, dict_data):
        self.source_data = dict_data
        self.data = bytes()
        self.type = -1

        #feature was sliced with grid, it will not overlap the boundary        
        self.sliced = "sliced" in dict_data["properties"]
        
        if dict_data["geometry"]["type"] == "Point":
            self.elevation = -32768
            self.name = ""
            lon, lat = dict_data["geometry"]['coordinates']

            if "name" in dict_data["properties"]:
                self.name = dict_data["properties"]["name"]
            if "ele" in dict_data["properties"]:
                self.elevation = int(float(dict_data["properties"]["ele"].replace(",", ".")))
            
            
            if "natural" in dict_data["properties"]:
                # 0 - peak
                if dict_data["properties"]["natural"] == "peak":
                    self.type = 0
                
            if "place" in dict_data["properties"]:
                # 10 - city
                # 11 - towen
                # 12 - vilage
                # 13 - hamlet, isolated_dwelling
                self.type = 13
                if dict_data["properties"]["place"] == "city":
                    self.type = 10
                if dict_data["properties"]["place"] == "town":
                    self.type = 11
                if dict_data["properties"]["place"] == "vilage":
                    self.type = 12
            
            name_lenght = len(self.name.encode("UTF-8"))
            assert(name_lenght <= 0xFF)
            
            if self.type != -1:
                #type
                self.data += struct.pack("B", self.type)
                #name lenght
                self.data += struct.pack("B", name_lenght)
                #elevation
                self.data += struct.pack("<h", self.elevation)
                #data
                self.data += struct.pack("<l", int(lon * common.GPS_COORD_MUL))
                self.data += struct.pack("<l", int(lat * common.GPS_COORD_MUL))
                self.data += self.name.encode("UTF-8")

                self.geometry = Point(lon, lat)

        if dict_data["geometry"]["type"] == "LineString":
            if "highway" in dict_data["properties"]:
                #   100 - motorway, trunk
                #   101 - primary
                #   102 - secondary, tertiary                
                self.type = 102
                if dict_data["properties"]["highway"] in ["motorway", "trunk"]:
                    self.type = 100
                if dict_data["properties"]["highway"] == "primary":
                    self.type = 101

            if "railway" in dict_data["properties"]:
                #   110 - rail
                if dict_data["properties"]["railway"] == "rail":
                    self.type = 110

            if "type" in dict_data["properties"]:
                if dict_data["properties"]["type"] == "river":
                #   120 - river
                    self.type = 120
            
            if "power" in dict_data["properties"]:
                #   130 - powerline
                self.type = 130
            if "aerialway" in dict_data["properties"]:
                #   131 - aerialway
                self.type = 131
                
            number_of_points = len(dict_data["geometry"]['coordinates'])
            assert(number_of_points > 0)
            assert(number_of_points <= 0xFFFF)

            if self.type != -1:
                #type
                self.data += struct.pack("B", self.type)
                #number od points
                self.data += struct.pack("<H", number_of_points)
                #data
                for pos in dict_data["geometry"]['coordinates']:
                    lon, lat = pos
                    self.data += struct.pack("<l", int(lon * common.GPS_COORD_MUL))
                    self.data += struct.pack("<l", int(lat * common.GPS_COORD_MUL))

                self.geometry = LineString(dict_data["geometry"]['coordinates'])
            

        if dict_data["geometry"]["type"] == "Polygon":
            #   200 - water
            #   201 - resident
            if "type" in dict_data["properties"]:            
                if dict_data["properties"]["type"] == "water":
                    self.type = 200

                if dict_data["properties"]["type"] == "resident":
                    self.type = 201

            if (len(dict_data["geometry"]['coordinates']) != 1):
                print(dict_data)
                tmp = Polygon(dict_data["geometry"]['coordinates'][0]).bounds
                print("rectangle name=error bbox=%f,%f,%f,%f" % tmp)
            
                assert(0)
                
                
            number_of_points = len(dict_data["geometry"]['coordinates'][0])
            assert(number_of_points > 0)
            assert(number_of_points <= 0xFFFF)

            if self.type != -1:
                #type
                self.data += struct.pack("B", self.type)
                #number od points
                self.data += struct.pack("<H", number_of_points)
                #data
                for pos in dict_data["geometry"]['coordinates'][0]:
                    lon, lat = pos
                    self.data += struct.pack("<l", int(lon * common.GPS_COORD_MUL))
                    self.data += struct.pack("<l", int(lat * common.GPS_COORD_MUL))

                self.geometry = Polygon(dict_data["geometry"]['coordinates'][0])      
            
            
        if self.type == -1:
            print(dict_data)
            raise Exception("Unable to parse feature")
            
    def inside(self, bbox):
        if self.sliced:
            return bbox.contains(self.geometry)
        else:
            return bbox.intersects(self.geometry)

    def get_data(self):
        return self.data

       
def pipeline_step4():    
    debug_bin = False
    debug_duplicates = False
   
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
    features_data = bytes()
    features_address = {}

    for i in range(w_cnt * h_cnt):
        grid.append([])

    print("Grid is %u x %u" % (w_cnt, h_cnt))

    features = []

    #list files
    files = os.listdir(source_dir)
    for filename in files:
        #skip grid
        if filename == "grid.geojson":
            continue
    
        source = os.path.join(source_dir, filename)
        print("Opening file: %s" % source)
        data = json.loads(open(source, "r").read())

        for dict_data in data["features"]:
            features.extend(feature_factory(dict_data))

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

                    if feature_index not in features_address:
                        features_address[feature_index] = len(features_data)
                        features_data += features[feature_index].get_data()

    print("done")

    index_records = 0
    index_contains = []
    duplicates = []
    for g in grid:
        index_records += len(g)     
        for i in g:   
            if i not in index_contains:
                index_contains.append(i)
            else:
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
        
    if len(duplicates) > 0 and debug_duplicates:            
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

    print("Index contains %u records (%u duplicates)" % (index_records,  len(duplicates)))

    file_data = bytes()

    if debug_bin:
        print("\n%08X *** writing header" % len(file_data))
    #header = 0
    # 8b id
    # 8b version
    # 8b grid w
    # 8b grid h
    file_data += struct.pack("<BBBB", 0x55, 0x00, w_cnt, h_cnt)

    # 32b timestamp
    timestamp = int(datetime.today().timestamp())
    file_data += struct.pack("<I", timestamp)
            
        
    if debug_bin:
        print("\n%08X *** writing grid info" % len(file_data))
        print("         X x Y: [index address], [number of features]")
    #grid = 8
    # 32b start address
    # 32b number of features
    #           header size + size of grid
    index_start_address = 8 + (8 * w_cnt * h_cnt)
    acc = 0;        
    for y in range(h_cnt):
        for x in range(w_cnt):
            grid_index = y * w_cnt + x
            addr = index_start_address + acc
            if debug_bin:
                print("%08X %u x %u: %X, %u" % (len(file_data), x, y, addr, len(grid[grid_index])))
            file_data += struct.pack("<II", addr, len(grid[grid_index]))
            acc += 4 * len(grid[grid_index])
            
            
    #index = 8 + 8 * w * h
    # 32b feature address  
    if debug_bin:
        print("\n%08X *** writing index" % len(file_data))
        print("         [feature index]: [feature address]")
    features_stat_address = index_start_address + index_records * 4
    for segment in grid:
        for g in segment:
            addr = features_address[g] + features_stat_address
            if debug_bin:
                print("%08X %4u: %X" % (len(file_data), g, addr))
            file_data += struct.pack("<I", addr)
        
        
    #features
    if debug_bin:
        print("\n%08X *** writing features" % len(file_data))
    file_data += features_data
    
    if debug_bin:
        print("\n%08X *** EOF" % len(file_data))

    f = open(target, "wb")
    f.write(file_data)
    f.close()

    print("total size %0.2fkB (%u b)" % (len(file_data) / 1024.0, len(file_data)))
    
if __name__ == '__main__':  
    pipeline_step4()    
