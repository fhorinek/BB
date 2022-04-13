# Logging for pilots

There are bugs that are hard to replicated based on the problem description, or does only occures under specific conditions. That is why we sometimes ask for the user cooperation to provide us with the log file. Log files describe device status and can provide more informations for us to locate the source of the problem.

Strato can output the debug informations to the file debug.log, via right USB port using virtual serial line or via hardware serial line, accesible only on development kits.

## How to enable debug.log
1. Go to the `System` / `Advanced` and enable `Debug to file`
2. Replicate the issue
3. Transfer the `debug.log` file from strato and send it to us (most likely fero@skybean.eu) with link to the issue or the problem description.

## Create diagnostic report (from R261.0.0)
Crash report enable us to view the internal state of the strato system. It porvide us with much more information. Normaly the Crash report is automaticali created during hard errors (device crash), but thay can be usefull for us with minor errors as well.
1. Go to the `System` / `Advanced` and enable `Debug to file`
2. If `Debug to file` is enabled `Clear debug.log` before proceding further
3. Restart the device and replicate the issue
4. Go to the `System` / `Advanced` and select `Create diagnostic report`
5. Device will go into safe mode and create crash report
6. Transfer the `crash_report.zip` file from strato and send it to us (most likely fero@skybean.eu) with link to the issue or the problem description.


# Logging for developers

If you want to enable logging statements into your source code, then
please start by setting the DEBUG_LEVEL that you want to use in your
source code. Place this line it before any `#include` statements
```c
    #define DEBUG_LEVEL DBG_DEBUG
```
You can also set to different levels (`DBG_INFO`, `DBG_WARN`, `DBG_ERROR`)
which will output every logging statement, that is equal or more
important than the defined level. Default is `DBG_INFO`.

Please use the `DBG()` statement to output your debug message. The
usage is identical to `printf`. 
Other statements for different levels are also avalible e.g.
```c
    //debuggin messages
    DBG("X/Y %d/%d", x, y);
    //info messages
    INFO("I am donig something!");
    //minor issue
    WARN("File not found, err = %u", err);
    //major issue
    ERR("Something terrible happened here!");
```
Asserts are also avalible
```c    
    //assert will output filename and line, if the condition is false
    ASSERT(state = HAL_OK);
    //fatal assert will trigger crash and output filename and line, if the condition is false
    FASSERT(allocated_memory != NULL);
``` 
    



