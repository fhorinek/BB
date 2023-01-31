# Map generation pipeline

Map data are generated from OSM data using local overpass server.

**Do not use pulic overpass servers, the request are too big and you will be blacklisted almost immediately**

## One time initialization

This has to be done only once. If done, then proceed to next chapter

```
root# docker run \
  -e OVERPASS_META=yes \
  -e OVERPASS_MODE=clone \
  -e OVERPASS_DIFF_URL=https://planet.openstreetmap.org/replication/day/ \
  -v /big_disk/overpass/:/db \
  -p 12346:80 \
  -i -t \
  --name overpass_world \
  wiktorn/overpass-api

root# docker start overpass_world
```

You will need several GB of space avalible, if you want to generate whole world
 * 3 arc seconds AGL data ~75GB
 * Overpass data for whole world ~272GB
 * Temporary data to generate tiles ~312GB 
 * Final dist packages for MAP and AGL files ~20GB

You will also need HGT data. They are only used to create AGL pack to complement the MAP files.
For map generation 3 arc second tiles are used.

Install osmtogeojson and mapshaper with npm
````sh
root# npm install -g osmtogeojson
root# npm install -g mapshaper
````

## Scripts
Map generations are done in several steps

 1. set correct path in common.py and make sure, that `storage_path` and `hgt_path` exist.
 
```python
storage_path = "/big_disk/OSM/"
hgt_path = "/big_disk/HGT3/data/hgt/"
overpass_url = "http://192.168.200.2:12346/api/interpreter"
```

 2. **download-mapshaper-data.py** - Download large mapshaper script data
 3. **get_countries.py** - download list of countries
 4. **get_borders.py** - download shapefiles for borders, convert, merge and simplify for tile generation
 5. **get_tiles.py** - create tile lists for countries
 6. **build.py [country list]** -create tiles for country *E.g. ./build.py DE_Germany*
 7. **pack.py [country list]** -create zip for map and agl tiles *E.g. ./build.py /big_disk/OSM/countries/DE_Germany.list*
 
 **status.py** - shows tiles done for countries, it will also create zip files when the country is done

 **update.py** - can be used instead of **build.py** to generate only a single tile, e.g. `./update.py N48E009`.

## How map generations works, how to change

The most important part is `build.py` which calls `update.py` for each
tile of a country.

`update.py` uses various scripts in `osm_scripts` and
`mapshaper_scripts` directory. Each deals with a specific part of the
OSM data and how to handle them. They are used by `step1.py` to
`step4.py` where the data gets transformed.

  1. step1.py - This script is for getting osm data for a tile by
     using all osm_scripts to query overpass server for data.

  2. step2.py - converts the result from step1 from osmjson to geojson
     format.

  3. step3.py - uses the mapshaper_scripts to convert the result from
     step2 into a more simple form. E.g. it reduces the number of
     points for a polygon to simplify the line.

  4. step4.py - This script convert pre-processed data from step3 to
     binary format for BB. It uses config/layer_order to layer all the
     previous separate data on top of each other. The last entry in
     the layer_order will be put on to the front and all others behind
     this. Example: if "towns" is after "water", then water will not
     be visible if crossing a town, because it is under the town and
     therefore invisible.

If you want to improve map generation by adding additional elements to
the map, then please do the following:

1. Select a good name for your elements, e.g. "hospital" to map all
   hospitals.

2. Add a script with this name to osm_scripts and mapshaper_scripts to
   extract and simplify the OSM data.

3. Add the same name to "config/layer_order" at the right place.

4. Add your new MAP_TYPES to ../../../BB3/App/gui/map/map_types.h and
   issue "make" here to generate MAP_TYPES.py

5. Extend step4.py to handle your new OSM features to map them to the
   new MAP_TYPES.

6. Execute "build.py" to generate a tile, copy it to Strato and extend
   map_obj.c and tile.c to handle your MAP_TYPES.

