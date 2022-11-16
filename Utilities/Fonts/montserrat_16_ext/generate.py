#!/usr/bin/python3

import os

size = 16 #16 default for text/icon font in strato

rel_path = "../../../BB3/App/gui/fonts/"

cmd = "lv_font_conv"
cmd += " --bpp 4"           
cmd += " --size %u" % size   
cmd += " --format lvgl"

#text
cmd += " --font Montserrat-Medium.ttf"
cmd += " -r 0x0020-0x007F"  # Basic Latin
cmd += " -r 0x00A0-0x00FF"  # Latin-1 Supplement
cmd += " -r 0x0100-0x017F"  # Latin Extended-A
cmd += " -r 0x0180-0x024F"  # Latin Extended-B

#LVGL icons
cmd += " --font FontAwesome5-Solid+Brands+Regular.woff"
cmd += " -r 61441,61448,61451,61452,61452,61453,61457,61459,61461,61465"
cmd += " -r 61468,61473,61478,61479,61480,61502,61512,61515,61516,61517"
cmd += " -r 61521,61522,61523,61524,61543,61544,61550,61552,61553,61556"
cmd += " -r 61559,61560,61561,61563,61587,61589,61636,61637,61639,61671"
cmd += " -r 61674,61683,61724,61732,61787,61931,62016,62017,62018,62019"
cmd += " -r 62020,62087,62099,62212,62189,62810,63426,63650"

#Additional icons scaled to 120% to match icons in FA
cmd += " --font materialdesignicons-webfont_120.woff"

header = "#ifndef MD_ICONS_H_\n"
header += "#define MD_ICONS_H_\n\n"

codes = []
for line in open("icons.txt", "r").readlines():
    line = line.split()
    if len(line) == 2:
        label = "MD_" + line[0].upper().replace("-", "_")
        code = int(line[1], 16)
        utf8 = chr(code).encode("utf-8")
        
        value = ""
        for c in utf8:
            value += "\\x%02x" % c
        
        header += "#define %-20s\t\"%s\"\n" % (label, value)
        codes.append("%u" % code)

cmd += " -r " + ",".join(codes)
header += "\n#endif\n"

f = open(rel_path + "md_icons.h", "w")
f.write(header)
f.close()

output = "%slv_font_montserrat_%u_ext.c" % (rel_path, size)

cmd += " -o %s" % output 

os.system(cmd)

#hacky way to fix geometry
data = open(output, "r").read()
data = data.replace(".line_height = 24,", ".line_height = 18,")
data = data.replace(".base_line = 5,", ".base_line = 3,")

f = open(output, "w")
f.write(data)
f.close()




