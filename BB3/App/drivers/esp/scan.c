#define DEBUG_LEVEL DEBUG_DBG

#include "scan.h"

#include "drivers/esp/esp.h"
#include "drivers/esp/protocol.h"

static scan_network_info_t * esp_scan_list = NULL;
static uint8_t scan_index = 0;
static uint32_t scan_auto_timer = 0;

#define SCAN_TIMEOUT    (60 * 1000)

void esp_scan_cb(proto_wifi_scan_res_t * network)
{
    if (network == NULL) //scan end
    {
        uint8_t best_index = 0xFF;

        for (uint8_t i = 0; i < scan_index; i++)
        {
            if (db_exists(PATH_NETWORK_DB, esp_scan_list[i].ssid))
            {
                if (best_index == 0xFF)
                    best_index = i;

                if (esp_scan_list[best_index].rssi < esp_scan_list[i].rssi)
                    best_index = i;
            }
        }

        if (best_index != 0xFF)
        {
            char pass[PROTO_WIFI_PASS_LEN];

            db_query(PATH_NETWORK_DB, esp_scan_list[best_index].ssid, pass, sizeof(pass));
            esp_wifi_connect(esp_scan_list[best_index].mac, esp_scan_list[best_index].ssid, pass, esp_scan_list[best_index].ch);
        }

        scan_auto_timer = HAL_GetTick() + SCAN_TIMEOUT;

        scan_index = 0;
        return;
    }

    if (scan_index > SCAN_NUMBER_OF_NETWORKS)
        return;

    esp_scan_list[scan_index].ch = network->ch;
    safe_memcpy(esp_scan_list[scan_index].mac, network->mac, 6);
    esp_scan_list[scan_index].rssi = network->rssi;
    esp_scan_list[scan_index].security = network->security;
    strncpy(esp_scan_list[scan_index].ssid, network->name, PROTO_WIFI_SSID_LEN);

    scan_index++;

}

void esp_scan_init()
{
	if (esp_scan_list == NULL)
	{
		esp_scan_list = (scan_network_info_t *)ps_malloc(sizeof(scan_network_info_t) * SCAN_NUMBER_OF_NETWORKS);
	}
    scan_index = 0;
    scan_auto_timer = 0;
}


void esp_scan_step()
{
    if (!config_get_bool(&config.wifi.autoconnect))
        return;

    if (!(fc.esp.state & ESP_STATE_WIFI_CLIENT))
    {
        scan_auto_timer = HAL_GetTick() + 1000;
        return;
    }

    if (fc.esp.state & ESP_STATE_WIFI_CONNECTED)
        return;

    if (esp_wifi_scanning())
        return;

    if (scan_auto_timer < HAL_GetTick())
    {
        esp_wifi_start_scan(esp_scan_cb);
    }
}
