/*
 * actions.inc
 *
 * Here you can define new shortcut actions
 *
 * Create include file named by the actions e.g. flight.inc
 *
 * Every action need two functions, one for action:
 *
 * static bool void shrt_<name>_action()
 * {
 *   <Code to execute>
 *   <done in scope of gui_thread, no locking of gui required>
 *
 *   return <true if you want to display label after the execution>;
 * }
 *
 * and one for icon and label:
 *
 * static bool shrt_<name>_icon(char * icon, char * label)
 * {
 *   strcpy(icon, <shortcut icon>);
 *   strcpy(label, <shortcut name>);
 *
 *   return <true if you want to add it to the shortcut selector>;
 * }
 *
 * then register the action:
 *
 * REGISTER_ACTION(<name>) //max len of <name> is SHORTCUT_NAME_LEN
 *
 *
 * To add icon go to Utilities/Fonts/montserrat_16_ext:
 *
 * Example: add bluetooth-off icon
 *
 * 1. Go to https://materialdesignicons.com/ and search for "bluetooth"
 * 2. Find name of the icon 'bluetooth-off'
 * 3. Get the utf value for the icon. I am using FontForge (Edit->Select->Select by Wildcard)
 * 4. Click on the icon, write down U+F00B2 => code is 0xF00B2
 *
 * add your icon to icons.txt
 * bluetooth-off        0xF00B2
 *
 * install lv_font_conv tool
 * > npm i lv_font_conv -g
 *
 * regenerate font (working directory Utilities/Fonts/montserrat_16_ext)
 * > ./generate.py
 */



//Actions include list

//add_left, add_right, no_action, dummy
#include "dummy.inc"

//wifi_tgl, bt_tgl
#include "wireless.inc"

//map_zoom_plus, map_zoom_mins
#include "map.inc"

//flight_start
#include "flight.inc"

//Actions register list
shortcut_item_t shortcut_actions[] = {
    //add_left and add_right must stay at the top
    REGISTER_ACTION(add_left),
    REGISTER_ACTION(add_right),
    REGISTER_ACTION(no_action),

    REGISTER_ACTION(wifi_tgl),
    REGISTER_ACTION(bt_tgl),

    REGISTER_ACTION(map_zoom_in),
    REGISTER_ACTION(map_zoom_out),

    REGISTER_ACTION(flight_start),
};
