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

void bt_a2d_cb(esp_a2d_cb_event_t event, esp_a2d_cb_param_t *param)
{
    INFO("A2DP %u", event);

    if (event == ESP_A2D_CONNECTION_STATE_EVT)
    {
        if (param->conn_stat.state == ESP_A2D_CONNECTION_STATE_CONNECTED)
        {
            bt_notify(param->conn_stat.remote_bda, "", PROTO_BT_MODE_CONNECTED | PROTO_BT_MODE_A2DP);
        }

        if (param->conn_stat.state == ESP_A2D_CONNECTION_STATE_DISCONNECTED)
        {
            bt_notify(param->conn_stat.remote_bda, "", PROTO_BT_MODE_DISCONNECTED | PROTO_BT_MODE_A2DP);
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

void bt_a2dp_init()
{
    pipe_bluetooth_init();

    //add callbacks
    esp_a2d_register_callback(bt_a2d_cb);
    esp_avrc_ct_register_callback(bt_avrc_ct_cb);
    esp_avrc_tg_register_callback(bt_avrc_tg_cb);

    //esp_hf_client_register_callback(bt_hf_client_cb);
    //esp_hf_client_init();
}
