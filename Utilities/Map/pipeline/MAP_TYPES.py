# Generated by h2py from ../../../BB3/App/gui/map/map_types.h
MAP_TYPE_POI_PEAK = 0
MAP_TYPE_POI_CITY = 10
MAP_TYPE_POI_TOWN = 11
MAP_TYPE_POI_VILLAGE = 12
MAP_TYPE_POI_HAMLET = 13
MAP_TYPE_POI_AEROWAY = 50
MAP_TYPE_POI_TAKEOFF = 51
MAP_TYPE_POI_LANDING = 52
MAP_TYPE_LINE_BORDER = 100
MAP_TYPE_LINE_RIVER = 110
MAP_TYPE_LINE_HIGHWAY = 120
MAP_TYPE_LINE_PRIMARY = 121
MAP_TYPE_LINE_SECONDARY = 122
MAP_TYPE_LINE_TERTIARY = 123
MAP_TYPE_LINE_RAIL = 130
MAP_TYPE_LINE_POWER = 140
MAP_TYPE_LINE_AERIALWAY = 141
MAP_TYPE_POLYGON_WATER = 200
MAP_TYPE_POLYGON_RESIDENT = 201
MAP_TYPE_POLYGON_AEROWAY = 202
MAP_TYPE_POLYGON_TAKEOFF = 203
MAP_TYPE_POLYGON_LANDING = 204
def MAP_TYPE_IS_POI(x): return (x/100==0)

def MAP_TYPE_IS_LINE(x): return (x/100==1)

def MAP_TYPE_IS_POLYGON(x): return (x/100==2)

