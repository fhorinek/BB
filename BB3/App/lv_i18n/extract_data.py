#!/usr/bin/python3

import os

origin = open("lv_i18n.c", "r").read()

origin = origin[origin.find("static lv_i18n_phrase_t de_de_singulars[] = {"):]
origin = origin[:origin.rfind("////////////////////////////////////////////////////////////////////////////////")]

os.remove("lv_i18n.c")
os.remove("lv_i18n.h")

with open("lv_i18n_data.inc", 'w') as f:
    f.write("\n")
    f.write(origin)
