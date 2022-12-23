#include "wifi_info.h"

#include "wifi.h"

#include "gui/gui_list.h"
#include "fc/fc.h"
#include "etc/format.h"

REGISTER_TASK_IL(wifi_info,
    lv_obj_t * sta_ip;
    lv_obj_t * sta_mac;
    lv_obj_t * ap_ip;
    lv_obj_t * ap_mac;
);

static void wifi_info_loop()
{
    char tmp[32];

    format_ip(tmp, fc.esp.ip_sta);
    gui_list_info_set_value(local->sta_ip, tmp);

    format_mac(tmp, fc.esp.mac_sta);
    gui_list_info_set_value(local->sta_mac, tmp);

    format_ip(tmp, fc.esp.ip_ap);
    gui_list_info_set_value(local->ap_ip, tmp);

    format_mac(tmp, fc.esp.mac_ap);
    gui_list_info_set_value(local->ap_mac, tmp);
}

static lv_obj_t * wifi_info_init(lv_obj_t * par)
{
    help_set_base("Wifi/Info");

	lv_obj_t * list = gui_list_create(par, _("Network info"), &gui_wifi, NULL);

    local->sta_ip = gui_list_info_add_entry(list, _h("Client IP"), "");
    local->sta_mac = gui_list_info_add_entry(list, _h("Client MAC"), "");
    local->ap_ip = gui_list_info_add_entry(list, _h("Access point IP"), "");
    local->ap_mac = gui_list_info_add_entry(list, _h("Access point MAC"), "");

	return list;
}



