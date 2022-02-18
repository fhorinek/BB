#!/usr/bin/python

import sys
import os
import common
from threading import Thread
import time

import step1
import step2
import step3
import step4

THREADS_CNT = 14

def process_tile(f):
    print("[%s] Start processing" % f)
    try:
        os.system("./update.py %s" % f)
    except Exception as e:
        print("***************************************************")
        l = open("errors.list", "a")
        l.write("%s\n" % f)
        l.write(str(e))
        l.write("\n\n")
        l.close()
        print("Error in processing tile %s" % f)
        print("\n", e,"\n")
        print("***************************************************")
    
    print("[%s] Done" % f)



if sys.argv[1] == "all":
    countries = os.listdir(common.target_dir_countries)
else:
    countries = [sys.argv[1] + ".list"]

for c in countries:
    path = os.path.join(common.target_dir_countries, c)
    files = open(path, "r").read().split("\n")

    threads = []
    for f in files:
        thread = Thread(target=process_tile, args=[f])
        thread.start()
        
        threads.append(thread)
        while len(threads) >= THREADS_CNT:
            for t in threads:
                if not t.is_alive():
                    threads.remove(t)
            
            time.sleep(0.05)
            

    
    
    
    

