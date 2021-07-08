
#define DEBUG_LEVEL DEBUG_DBG

#include <gui/tasks/menu/wifi/wifi_list.h>

#include <gui/tasks/menu/wifi/wifi.h>

#include "gui/gui_list.h"
#include "drivers/esp/esp.h"
#include "drivers/esp/protocol.h"

#include "config/db.h"
#include "config/config.h"
#include "gui/dialog.h"

typedef struct
{
    uint8_t mac[6];
    int8_t rssi;
    uint8_t security: 4;
    uint8_t ch: 4;
}
network_info_t;

#define NUMBER_OF_NETWORKS  16

REGISTER_TASK_I(wifi_list,
    lv_obj_t * spinner;
    lv_obj_t * info;

    network_info_t nets[NUMBER_OF_NETWORKS];

    uint8_t selected;
    uint8_t size;
    bool new_scan;
);

void wifi_list_update(proto_wifi_scan_res_t * network)
{
    if (network == NULL)
    {
        local->new_scan = true;
        return;
    }

    gui_lock_acquire();

#if DEBUG_LEVEL <= DBG_DEBUG
    char mac[3 * 6 + 1];
    format_mac(mac, network->mac);
    DBG("%s '%s' %i dBm %u", mac, network->name, network->rssi, network->security);
#endif

    if (local->new_scan)
    {
        local->new_scan = false;
        local->size = 0;

        for (uint8_t i = 1; ; i++)
        {
            lv_obj_t * item = gui_list_get_entry(1);
            if (item == NULL)
                break;
            lv_obj_del(item);
        }

        lv_obj_set_hidden(local->spinner, true);
        gui_list_text_set_value(local->info, "Rescan");
    }


    lv_obj_t * entry = NULL;

    for (uint8_t i = 0; i < local->size; i++)
    {
        lv_obj_t * item = gui_list_get_entry(i + 1);
        if (strcmp(gui_list_info_get_name(item), network->name) == 0 && local->nets[i].security == network->security)
        {
            //it is the same network
            if (local->nets[i].rssi > network->rssi)
            {
                //stored have better signal
                gui_lock_release();
                return;
            }
            else
            {
                //new have better signal
                memcpy(local->nets[i].mac, network->mac, 6);
                local->nets[i].rssi = network->rssi;
                local->nets[i].ch = network->ch;
                entry = item;
                break;
            }
        }
    }

    char params[32] = "";

    const char * saved = (db_exists(PATH_NETWORK_DB, network->name)) ? "Saved, " : "";
    const char * secur = (network->security == PROTO_WIFI_OPEN) ? "Open" : "Secure";

    sprintf(params, "%s%s, %d dBm", saved, secur, network->rssi);

    if (entry == NULL)
    {
        entry = gui_list_info_add_entry(gui.list.object, network->name, params);

        //new
        memcpy(local->nets[local->size].mac, network->mac, 6);
        local->nets[local->size].rssi = network->rssi;
        local->nets[local->size].security = network->security;
        local->nets[local->size].ch = network->ch;
        local->size++;
    }
    else
    {
        for (uint8_t i = 0; i < 3; i++)
        {
            lv_obj_del(lv_obj_get_child(entry, NULL));
        }
        gui_list_info_set_value(entry, params);
    }

    uint8_t bars = 0;
    lv_color_t color;
    if (network->rssi >= -60)
    {
        bars = 3;
        color = LV_COLOR_GREEN;
    }
    else if (network->rssi >= -67)
    {
        bars = 2;
        color = LV_COLOR_YELLOW;
    }
    else
    {
       if (network->rssi >= -70)
            bars = 1;
        else
            bars = 0;

       color = LV_COLOR_RED;
    }

    for (uint8_t i = 1; i <= 3; i++)
    {
        lv_obj_t * bar = lv_obj_create(entry, NULL);
        lv_obj_set_size(bar, 6, (i <= bars) ? (i * 6) : 1);
        lv_obj_align(bar, entry, LV_ALIGN_IN_TOP_RIGHT, -5 * 2 * (4 - i), 24 - lv_obj_get_height(bar));
        lv_obj_set_style_local_bg_color(bar, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, color);
    }

    gui_lock_release();
}

void wifi_list_connect_cb(uint8_t res, void * data)
{
    char * pass = data;
    if (res == dialog_res_yes)
    {
        char * ssid = gui_list_info_get_name(gui_list_get_entry(local->selected + 1));
        esp_wifi_connect(local->nets[local->selected].mac, ssid, pass, local->nets[local->selected - 1].ch);
        gui_switch_task(&gui_wifi, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
    }
}

static bool wifi_list_cb(lv_obj_t * obj, lv_event_t event, uint8_t index)
{
	if (event == LV_EVENT_CLICKED)
	{
	    if (obj == local->info)
	    {
	        local->new_scan = true;
	        gui_list_text_set_value(local->info, "Scanning...");
	        esp_wifi_start_scan(wifi_list_update);
	    }
	    else
	    {
	        char * ssid = gui_list_info_get_name(obj);
//	        gui_switch_task(&gui_wifi, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
	        if (local->nets[index - 1].security == PROTO_WIFI_OPEN)
	        {
	            esp_wifi_connect(local->nets[index - 1].mac, ssid, "", local->nets[index - 1].ch);
	        }
	        else
	        {
	            char pass[64];


	            bool found = db_query(PATH_NETWORK_DB, ssid, pass, sizeof(pass));
	            if (found)
	            {
	                esp_wifi_connect(local->nets[index - 1].mac, ssid, pass, local->nets[index - 1].ch);
	                gui_switch_task(&gui_wifi, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
	            }
	            else
	            {
	                //ask for password
	                local->selected = index - 1;
	                dialog_show(ssid, "Enter password", dialog_textarea, wifi_list_connect_cb);
	            }
	        }
	    }
	}
	return true;
}


static lv_obj_t * wifi_list_init(lv_obj_t * par)
{
    local->new_scan = true;
    esp_wifi_start_scan(wifi_list_update);

	lv_obj_t * list = gui_list_create(par, "Select network", &gui_wifi, wifi_list_cb);

    local->spinner = lv_spinner_create(par, NULL);
    lv_obj_set_size(local->spinner, 100, 100);
    lv_obj_align(local->spinner, NULL, LV_ALIGN_CENTER, 0, 0);

    local->info = gui_list_text_add_entry(list, "Scanning...");

	return list;
}

