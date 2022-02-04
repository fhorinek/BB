# Logging for pilots

There are bugs that are hard to replicated based on the problem description, or does only occures under specific conditions. That is why we sometimes ask for the user cooperation to provide us with the log file. Log files describe device status and can provide more informations for us to locate the source of the problem.

Strato can output the debug informations to the file debug.log, via right USB port using virtual serial line or via hardware serial line, accesible only on development kits.

## Enable debug mode and create debug.log
To acces the debug option you need to enable specia development menu.
1. Go to `System` -> `Device info` -> `Serial number`, press the `Serial number` 6 times to enable or disable the Development menu
2. Now you can see a new sub-menu in setting called `Development`
3. Go to the `Development` and enable `Debug to file`
4. Replicate the issue
5. Transfer the debug.log file from strato and send it to us (most likely fero@skybean.eu) with link to the issue or the problem description.

# Logging for developers

If you want to enable logging statements into your source code, then
please start by setting the DEBUG_LEVEL that you want to use in your
source code

    #define DEBUG_LEVEL DBG_DEBUG

You can also set to different levels (`DBG_INFO, DBG_WARN, DBG_ERROR`)
which will output every logging statement, that is equal or more
important than the defined level.

Please use the `DBG()` statement to output your debug message. The
usage is identical to `printf`, e.g.

    DBG("X/Y %d/%d", x, y);



