/*
 * wifi.c
 *
 *  Created on: 17. 3. 2021
 *      Author: horinek
 */


#include "wifi.h"

#include "esp_wifi.h"
#include "protocol.h"


void wifi_init()
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
}

void wifi_enable(bool client, bool ap)
{
	wifi_mode_t mode;

	if (client && !ap)
		mode = WIFI_MODE_STA;
	else if (client && ap)
		mode = WIFI_MODE_APSTA;
	else if (!client && ap)
		mode = WIFI_MODE_AP;
	else
		mode = WIFI_MODE_NULL;

	if (mode != config.wifi_mode)
	{
		if (config.wifi_mode != WIFI_MODE_NULL)
			esp_wifi_stop();

		if (mode != WIFI_MODE_NULL)
		{
			esp_wifi_set_mode(mode);
			esp_wifi_start();
		}

		config.wifi_mode = mode;
	}
}

#define DEFAULT_SCAN_LIST_SIZE	16

void wifi_start_scan()
{
	if (config.wifi_mode == WIFI_MODE_STA || config.wifi_mode == WIFI_MODE_APSTA)
	{
		uint16_t number = DEFAULT_SCAN_LIST_SIZE;
		wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
		uint16_t ap_count = 0;

	    esp_wifi_scan_start(NULL, true);
	    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
	    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));

	    for (uint16_t i = 0; (i < DEFAULT_SCAN_LIST_SIZE) && (i < ap_count); i++)
	    {
	    	proto_wifi_scan_res_t res;
	    	strncpy(res.name, (char *)ap_info[i].ssid, 32);

	    	protocol_send(PROTO_WIFI_SCAN_RES, (void *)&res, sizeof(res));
	    }
	}
}

