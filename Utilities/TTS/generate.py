#!/usr/bin/python

import os
import shutil
import sys

from phrases import phrases

base_path = os.path.dirname(os.path.realpath(__file__)) + "/../../BB3/"
assets_path = os.path.join(base_path, "Assets", "tts")
if not os.path.exists(assets_path):
    os.mkdir(assets_path)

for f in os.listdir(os.path.dirname(os.path.realpath(__file__))):
    if f[0] == "_" or f[0] == ".":
        continue

    if os.path.isdir(f):
        print("Processing %s" % f)
    
        sys.path.append(f)
        language = __import__("%s_phrases" % f)
        
        lang_dir = os.path.join(assets_path, f)
        if os.path.exists(lang_dir):
            shutil.rmtree(lang_dir)
            
        os.mkdir(lang_dir)
        for phrase in phrases:
            p = os.path.join(lang_dir, phrase)
            cmd = language.command % (language.phrases[phrase], p)
            
            print("  %s\t%s" % (phrase, cmd))
            os.system(cmd)
    
        
        

