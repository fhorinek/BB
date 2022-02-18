# Map generation pipeline

Map data are generated from OSM data using local overpass server.

**Do not use pulic overpass servers, the request are too big and you will be blacklisted almost immediately**

```
docker run \
  -e OVERPASS_META=yes \
  -e OVERPASS_MODE=clone \
  -e OVERPASS_DIFF_URL=https://planet.openstreetmap.org/replication/day/ \
  -v /big_disk/overpass/:/db \
  -p 12346:80 \
  -i -t \
  --name overpass_world \
  wiktorn/overpass-api
```

You will need several GB of space avalible, if you want to generate whole world
 * 3 arc seconds AGL data ~75GB
 * Overpass data for whole world ~272GB
 * Temporary data to generate tiles ~312GB 
 * Final dist packages for MAP and AGL files ~20GB

You will also need HGT data. They are only used to create AGL pack to complement the MAP files.
For map generation 3 arc second tiles are used.

## Scripts
Map generations are done in several steps

 1. set correct path in common.py 
``` 
storage_path = "/big_disk/OSM/"
hgt_path = "/big_disk/HGT3/data/hgt/"
overpass_url = "http://192.168.200.2:12346/api/interpreter"
```
 2. **get_countrie.py** - download list of countries
 3. **get_borders.py** - download shapefiles for borders, convert, merge and simplify for tile generation
 4. **get_tiles.py** - create tile lists for countries
 5. **build.py [country list]** -create tiles for country *E.g. ./build.py /big_disk/OSM/countries/DE_Germany.list*
 6. **pack.py [country list]** -create zip for map and agl tiles *E.g. ./build.py /big_disk/OSM/countries/DE_Germany.list*
 
 **status.py** - shows tiles done for countries

