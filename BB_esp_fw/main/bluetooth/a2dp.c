/*
 * a2dp.c
 *
 *  Created on: 4. 10. 2021
 *      Author: horinek
 */

#include "a2dp.h"

#include "pipeline/bluetooth.h"
#include "bluetooth.h"

#include "esp_a2dp_api.h"
#include "esp_avrc_api.h"
#include "esp_hf_client_api.h"

#define A2DP_CONN_TIMEOUT		(5 * 1000)

static esp_bd_addr_t last_a2dp_device = {0};
TimerHandle_t a2dp_reconnect_timer = 0;
static bool bt_a2dp_connected = false;

void bt_a2dp_reconnect_cb(TimerHandle_t xTimer)
{
	esp_bd_addr_t null_adr = {0};

	if (memcmp(last_a2dp_device, null_adr, sizeof(esp_bd_addr_t)) != 0)
	{
		INFO("trying to connect to the last device");
		esp_a2d_sink_connect(last_a2dp_device);
	}
}



void bt_a2d_cb(esp_a2d_cb_event_t event, esp_a2d_cb_param_t *param)
{
    INFO("A2DP %u", event);

    if (event == ESP_A2D_CONNECTION_STATE_EVT)
    {
        if (param->conn_stat.state == ESP_A2D_CONNECTION_STATE_CONNECTED)
        {
        	xTimerStop(a2dp_reconnect_timer, WAIT_INF);
            bt_notify(param->conn_stat.remote_bda, "", PROTO_BT_MODE_CONNECTED | PROTO_BT_MODE_A2DP);
            memcpy(last_a2dp_device, param->conn_stat.remote_bda, sizeof(esp_bd_addr_t));
            bt_a2dp_connected = true;
        }

        if (param->conn_stat.state == ESP_A2D_CONNECTION_STATE_DISCONNECTED)
        {
        	if (bt_a2dp_connected)
        	{
        		bt_notify(param->conn_stat.remote_bda, "", PROTO_BT_MODE_DISCONNECTED | PROTO_BT_MODE_A2DP);
        		bt_a2dp_connected = false;
        		xTimerStart(a2dp_reconnect_timer, WAIT_INF);
        	}
        }
    }
}

void bt_avrc_ct_cb(esp_avrc_ct_cb_event_t event, esp_avrc_ct_cb_param_t *param)
{
    INFO("AVRC CT %u", event);
}

void bt_avrc_tg_cb(esp_avrc_tg_cb_event_t event, esp_avrc_tg_cb_param_t *param)
{
    switch (event)
    {
        case ESP_AVRC_TG_SET_ABSOLUTE_VOLUME_CMD_EVT:
        {
            INFO("set_abs_vol %u", param->set_abs_vol);
        }
        break;

        default:
            INFO("AVRC TG %u", event);
        break;
    }
}

void bt_a2dp_init(proto_mac_t autoconnect_dev)
{
    pipe_bluetooth_init();

    //add callbacks
    esp_a2d_register_callback(bt_a2d_cb);
    esp_avrc_ct_register_callback(bt_avrc_ct_cb);
    esp_avrc_tg_register_callback(bt_avrc_tg_cb);

    //esp_hf_client_register_callback(bt_hf_client_cb);
    //esp_hf_client_init();

    memcpy(last_a2dp_device, autoconnect_dev, sizeof(esp_bd_addr_t));

    a2dp_reconnect_timer = xTimerCreate("a2dp_con", A2DP_CONN_TIMEOUT / portTICK_PERIOD_MS, true, 0, bt_a2dp_reconnect_cb);
    xTimerStart(a2dp_reconnect_timer, WAIT_INF);
}
