/**
 * This defines the various map types which are used in the binary MAP files.
 * They must be in sync with the python scripts in Utilities/Map/pipeline.
 *
 * To keep them in sync, please issue
 *   shell% cd ../../../../Utilities/Map/pipeline/
 *   shell% make
 */

#ifndef __MAP_TYPES_H
#define __MAP_TYPES_H

// The following POI are presented as labels with their name attribute
#define MAP_TYPE_POI_PEAK 0
#define MAP_TYPE_POI_CITY 10
#define MAP_TYPE_POI_TOWN 11
#define MAP_TYPE_POI_VILLAGE 12
#define MAP_TYPE_POI_HAMLET 13

// The following POI are presented as icons
#define MAP_TYPE_POI_AEROWAY 50
#define MAP_TYPE_POI_TAKEOFF 51
#define MAP_TYPE_POI_LANDING 52

// The following MAP_TYPEs are presented as lines in different colors
#define MAP_TYPE_LINE_BORDER 100
#define MAP_TYPE_LINE_RIVER 110
#define MAP_TYPE_LINE_HIGHWAY 120
#define MAP_TYPE_LINE_PRIMARY 121
#define MAP_TYPE_LINE_SECONDARY 122
#define MAP_TYPE_LINE_TERTIARY 123
#define MAP_TYPE_LINE_RAIL 130
#define MAP_TYPE_LINE_POWER 140
#define MAP_TYPE_LINE_AERIALWAY 141

// The following MAP_TYPEs are presented as polygons in different colors
#define MAP_TYPE_POLYGON_WATER 200
#define MAP_TYPE_POLYGON_RESIDENT 201
#define MAP_TYPE_POLYGON_AEROWAY 202
#define MAP_TYPE_POLYGON_TAKEOFF 203
#define MAP_TYPE_POLYGON_LANDING 204

// Macros to check for different MAP_TYPES
#define MAP_TYPE_IS_POI(x) (x/100==0)
#define MAP_TYPE_IS_POI_LABEL(x) (x>0 && x<50)
#define MAP_TYPE_IS_POI_ICON(x) (x>=50 && x<60)
#define MAP_TYPE_IS_LINE(x) (x/100==1)
#define MAP_TYPE_IS_POLYGON(x) (x/100==2)

#endif


