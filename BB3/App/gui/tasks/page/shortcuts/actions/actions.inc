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
 */



//Actions include list

//add_left, add_right, no_action, dummy
#include "dummy.inc"
#include "wireless.inc"

//Actions register list
shortcut_item_t shortcut_actions[] = {
    REGISTER_ACTION(add_left),
    REGISTER_ACTION(add_right),
    REGISTER_ACTION(no_action),
    REGISTER_ACTION(dummy),

    REGISTER_ACTION(wifi_tgl),
};
