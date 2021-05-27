/*
 * wifi.c
 *
 *  Created on: 17. 3. 2021
 *      Author: horinek
 */
#define DEBUG_LEVEL DBG_DEBUG

#include "wifi.h"

#include "esp_wifi.h"
#include "mdns.h"
#include "protocol.h"

static wifi_mode_t wifi_mode = WIFI_MODE_NULL;

static esp_netif_t * sta_netif;
static char wifi_ssid[PROTO_WIFI_SSID_LEN];
static char wifi_pass[PROTO_WIFI_PASS_LEN];

static bool wifi_connected = false;
static uint8_t wifi_retry;

static uint8_t wifi_ap_clients = 0;

static esp_netif_t * ap_netif;

#define WIFI_CONNECTION_TRIES	10

void wifi_ip_events(void * null, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
	DBG("IP EVENT: %s %d", event_base, event_id);
	switch (event_id)
	{
		case(IP_EVENT_STA_GOT_IP):
		{
			ip_event_got_ip_t * edata = (ip_event_got_ip_t *)event_data;
			proto_wifi_got_ip_t data;
			memcpy(data.ip, (uint8_t *)&edata->ip_info.ip.addr, 4);
			memcpy(data.mask, (uint8_t *)&edata->ip_info.netmask.addr, 4);
			memcpy(data.gw, (uint8_t *)&edata->ip_info.gw.addr, 4);

			protocol_send(PROTO_WIFI_GOT_IP, (uint8_t *)&data, sizeof(data));
		}

		case(IP_EVENT_AP_STAIPASSIGNED):
		{
//			protocol_send(PROTO_WIFI_GOT_IP, (uint8_t *) data, sizeof(data));

		}

	}
}

void wifi_eth_events(void * null, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
	DBG("ETH EVENT: %s %d", event_base, event_id);

}

void wifi_wifi_events(void * null, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
	DBG("WIFI EVENT: %s %d", event_base, event_id);
	switch (event_id)
	{
		case(WIFI_EVENT_STA_START):
			protocol_send(PROTO_WIFI_ENABLED, NULL, 0);
		break;

		case(WIFI_EVENT_STA_STOP):
			protocol_send(PROTO_WIFI_DISABLED, NULL, 0);
		break;

		case(WIFI_EVENT_STA_CONNECTED):
		{
			proto_wifi_connected_t data;
			strncpy(data.ssid, wifi_ssid, sizeof(data.ssid));
			strncpy(data.pass, wifi_pass, sizeof(data.pass));

			protocol_send(PROTO_WIFI_CONNECTED, (uint8_t *)&data, sizeof(data));

			wifi_connected = true;
			wifi_retry = WIFI_CONNECTION_TRIES;
		}
		break;

		case(WIFI_EVENT_STA_DISCONNECTED):
		{
			wifi_event_sta_disconnected_t * edata = event_data;
			INFO("reason: %u", edata->reason);

			wifi_connected = false;

			if (wifi_retry > 0 && edata->reason != WIFI_REASON_ASSOC_LEAVE)
			{
				INFO("Trying to reconnect %u", wifi_retry);
				wifi_retry--;
				ESP_ERROR_CHECK(esp_wifi_connect());
			}
			else
			{
				protocol_send(PROTO_WIFI_DISCONNECTED, NULL, 0);
			}
		}
		break;

		case(WIFI_EVENT_AP_START):
		{
			esp_netif_ip_info_t ip_info;
			proto_wifi_ap_enabled_t data;

			esp_netif_get_ip_info(ap_netif, &ip_info);
			memcpy(data.ip, (uint8_t *)&ip_info.ip, 4);

			protocol_send(PROTO_WIFI_AP_ENABLED, (uint8_t *)&data, sizeof(data));
			wifi_ap_clients = 0;
		}
		break;

		case(WIFI_EVENT_AP_STOP):
			protocol_send(PROTO_WIFI_AP_DISABLED, NULL, 0);
			wifi_ap_clients = 0;
		break;

		case(WIFI_EVENT_AP_STACONNECTED):
		{
			wifi_ap_clients++;
			wifi_event_ap_staconnected_t * edata = event_data;
			proto_wifi_client_connected_t data;

			memcpy(data.mac, edata->mac, 6);
			protocol_send(PROTO_WIFI_AP_CONNETED, (uint8_t *)&data, sizeof(data));
		}
		break;

		case(WIFI_EVENT_AP_STADISCONNECTED):
			if (wifi_ap_clients > 0)
			{
				wifi_ap_clients--;

				if (wifi_ap_clients == 0)
					protocol_send(PROTO_WIFI_AP_DISCONNETED, NULL, 0);
			}
		break;
	}
}

void wifi_init()
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);
    ap_netif = esp_netif_create_default_wifi_ap();
    assert(ap_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_wifi_events, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, ESP_EVENT_ANY_ID, wifi_ip_events, NULL, NULL);
    esp_event_handler_instance_register(ETH_EVENT, ESP_EVENT_ANY_ID, wifi_eth_events, NULL, NULL);

    ESP_ERROR_CHECK(mdns_init());
    mdns_hostname_set(DEVICE_HOSTNAME);
    mdns_instance_name_set(DEVICE_INSTANCE);
    mdns_service_add(NULL, "_http", "_tcp", 80, NULL, 0);
}


void wifi_enable(proto_wifi_mode_t * packet)
{
	wifi_mode_t mode;

	if (packet->client && !packet->ap)
		mode = WIFI_MODE_STA;
	else if (packet->client && packet->ap)
		mode = WIFI_MODE_APSTA;
	else if (!packet->client && packet->ap)
		mode = WIFI_MODE_AP;
	else
		mode = WIFI_MODE_NULL;

	if (mode != wifi_mode)
	{
		wifi_config_t * wifi_config = ps_malloc(sizeof(wifi_config_t));

		ESP_ERROR_CHECK(esp_wifi_set_mode(mode));

		if (packet->ap)
		{
			strcpy((char *)wifi_config->ap.ssid, packet->ssid);
			wifi_config->ap.ssid_len = strlen(packet->ssid);
			wifi_config->ap.max_connection = 2;
			if (strlen(packet->pass) >= 8)
			{
				strcpy((char *)wifi_config->ap.password, packet->pass);
				wifi_config->ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
			}
			else
			{
				wifi_config->ap.authmode = WIFI_AUTH_OPEN;
			}

			esp_netif_set_hostname(ap_netif, DEVICE_HOSTNAME);
			ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, wifi_config));
		}

		if (packet->client)
		{
			memset(wifi_config, 0, sizeof(wifi_config_t));

			wifi_config->sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
			wifi_config->sta.listen_interval = 20;

			esp_wifi_set_config(WIFI_IF_STA, wifi_config);
			esp_netif_set_hostname(sta_netif, DEVICE_HOSTNAME);
		}

		if (mode != WIFI_MODE_NULL)
			ESP_ERROR_CHECK(esp_wifi_start());
		else
			ESP_ERROR_CHECK(esp_wifi_stop());

		wifi_mode = mode;

		free(wifi_config);
	}

	free(packet);
	vTaskDelete(NULL);
}

#define SCAN_LIST_SIZE	16

void wifi_start_scan(void * parameter)
{
	if (wifi_mode == WIFI_MODE_STA || wifi_mode == WIFI_MODE_APSTA)
	{
		uint16_t number = SCAN_LIST_SIZE;
		wifi_ap_record_t * ap_info = ps_malloc(sizeof(wifi_ap_record_t) * SCAN_LIST_SIZE);
		uint16_t ap_count = 0;

	    esp_wifi_scan_start(NULL, true);
	    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
	    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));

	    for (uint16_t i = 0; (i < SCAN_LIST_SIZE) && (i < ap_count); i++)
	    {
	    	proto_wifi_scan_res_t res;

	    	strncpy(res.name, (char *)ap_info[i].ssid, 31);
	    	memcpy(&res.mac, ap_info[i].bssid, 6);
	    	res.rssi = ap_info[i].rssi;
	    	res.security = ap_info[i].authmode;
	    	res.ch = ap_info[i].primary;

	    	protocol_send(PROTO_WIFI_SCAN_RES, (void *)&res, sizeof(res));
	    }

	    protocol_send(PROTO_WIFI_SCAN_END, NULL, 0);
	    free(ap_info);
	}

	vTaskDelete(NULL);
}

void wifi_connect(proto_wifi_connect_t * packet)
{
	if (wifi_mode == WIFI_MODE_STA || wifi_mode == WIFI_MODE_APSTA)
	{
		INFO("Connect to '%s' pw '%s' ch %u", packet->ssid, packet->pass, packet->ch);
		DUMP(packet->mac, 6);
		wifi_config_t * wifi_config = (wifi_config_t * )ps_malloc(sizeof(wifi_config_t));

		esp_wifi_get_config(WIFI_IF_STA, wifi_config);

		wifi_config->sta.bssid_set = false;
		for (uint8_t i = 0; i < 6; i++)
		{
			if (packet->mac[i] != 0x00)
			{
				wifi_config->sta.bssid_set = true;
				memcpy(wifi_config->sta.bssid, packet->mac, 6);
				break;
			}
		}

		strncpy((char *)wifi_config->sta.ssid, packet->ssid, sizeof(wifi_config->sta.ssid));
		strncpy((char *)wifi_config->sta.password, packet->pass, sizeof(wifi_config->sta.password));
		wifi_config->sta.channel = packet->ch;

		ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, wifi_config));

		//store ssid and pass
		strncpy(wifi_ssid, packet->ssid, sizeof(wifi_ssid));
		strncpy(wifi_pass, packet->pass, sizeof(wifi_pass));

		if (wifi_connected)
		{
			wifi_retry = 0;
			ESP_ERROR_CHECK(esp_wifi_disconnect());
		}

		wifi_connected = false;
		wifi_retry = WIFI_CONNECTION_TRIES;
		ESP_ERROR_CHECK(esp_wifi_connect());
		free(wifi_config);
	}


	free(packet);
	vTaskDelete(NULL);
}

