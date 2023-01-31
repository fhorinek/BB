#!/usr/bin/python3
#this script is for getting the 240 MB mapshaper script

import os
import common
import urllib.request

def download_data():
    url = "https://strato.skybean.eu/dev/merged_24_c.shp"
    
    print("Downloading 240 MB from ", url)
    urllib.request.urlretrieve(url, common.target_dir_land_poly)
    
def download_mapshaper_data():
    if not os.path.isdir(common.target_dir_land_poly_dir):
        os.mkdirs(common.target_dir_land_poly_dir)
    if not os.path.isfile(common.target_dir_land_poly):
        download_data()
    else:
        print(common.target_dir_land_poly, "already downloaded")
        
if __name__ == '__main__':  
    download_mapshaper_data()
    
