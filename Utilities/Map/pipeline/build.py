#!/usr/bin/python

import sys
import os
import common

import step1
import step2
import step3
import step4

path = sys.argv[1]
files = open(path, "r").read().split("\n")

for f in files:

    print("Processing", f)
    lon, lat = common.filename_to_lon_lat(f)
    common.init_vars(lon, lat)

    try:
        step1.pipeline_step1()
        step2.pipeline_step2()
        step3.pipeline_step3()
        step4.pipeline_step4()
    except:
        print("***************************************************")
        l = open("errors.list", "a")
        l.write("%s\n" % f)
        l.close()
        print("Error in processing tile %s" % f)
        print("***************************************************")
        raise Exception("Error not ignored!")

