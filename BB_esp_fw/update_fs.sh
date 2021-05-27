./spiffsgen.py 1048576 spiffs_image fs.bin
esptool.py --chip esp32 --port /dev/ttyUSB1 write_flash -z 0xa10000 fs.bin
