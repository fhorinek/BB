# How to use CrashDebug

CrashDebug is a tool to load device crash dump into gdb.

Strato create crash dump (snapshot of memory and mcu registers) in event od Hard Fault.
To load thease files into the CubeIDE you need to configure new debug launcher.

1. Go to debug configurations
2. Create new GDB Hardware Debugging
3. Set parameters:
 * **main** / C/C++ Application : `${config_name:BB3}/BB3.elf`
 * **Debbuger** / GDB Command : `arm-none-eabi-gdb`
 * **Startup** / Initialization Commands : 
 ```
    set target-charset ASCII
    target remote | ${project_loc}/../Utilities/CrashDebug/lin64/CrashDebug --elf  ${config_name:BB3}/BB3.elf --dump ${project_loc}/../Utilities/CrashDebug/dump.bin
 ```
 * **Startup** / Load image : unchecked
4. Place `dump.bin` from the crash_report zip file here
5. Run the created launcher

Notes
* You need to have Crash reports enabled in System/Advanced
* You need to checkout the code for the debugged firmware, otherwise the lines will not match
* If you are on macOS, replace `lin64` with `osx64` above

