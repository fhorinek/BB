#!/usr/bin/python3

import subprocess
import shutil
import os
import yaml
import sys

if len(sys.argv) == 2:
    zip_file = sys.argv[1]
else:
    f = subprocess.check_output(["zenity", "--file-selection", "--file-filter=*.zip", "--title", "'Choose crash report bunde'"])
    zip_file = "".join(f.decode("utf-8").split())
 
shutil.rmtree("report", ignore_errors=True)
os.mkdir("report")
os.system("unzip %s -d report" % zip_file)

info = yaml.safe_load(open("report/crash/info.yaml", "r").read())
fw = info["firmware_version"]
print("Firmware version is: %s" % fw)


print("Getting copy of the code")
have_tag = len("".join(subprocess.check_output(["git", "tag", "-l", fw]).decode("utf-8").split())) > 0

if not have_tag:
    print("fw %s not found in repo" % fw)
    fw = "master"

if not os.path.exists("code/%s" % fw):
    shutil.rmtree("code", ignore_errors=True)
    os.system("GIT_LFS_SKIP_SMUDGE=1 git clone --branch %s ../../../ code" % fw)
    os.system("touch code/%s" % fw)      
    os.makedirs("code/BB3/Release", exist_ok=True)
    
print("Getting elf file")
os.makedirs("elf", exist_ok=True)

if have_tag:
    if not os.path.exists("elf/%s.elf" % fw):
        url = "https://strato.skybean.eu/metrics/elf/%s.elf" % fw
        os.system("wget %s -O elf/%s.elf" % (url, fw))
else:        
    if fw == "master":
        shutil.copy("../../../BB3/Release/BB3.elf", "elf/master.elf")

   
main_cmd = """
gdbgui -g "arm-none-eabi-gdb elf/%s.elf 
--eval-command 'directory code/BB3/Release'
--eval-command 'set target-charset ASCII' 
--eval-command 'target remote | ../lin64/CrashDebug --elf elf/%s.elf --dump report/crash/dump.bin'" 
""" % (fw, fw)
    
print(main_cmd)    
os.system(main_cmd)       


