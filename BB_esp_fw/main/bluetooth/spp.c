/*
 * spp.c
 *
 *  Created on: 4. 10. 2021
 *      Author: horinek
 */

#include "spp.h"
#include "bluetooth.h"
#include "protocol.h"
#include "esp_spp_api.h"


static uint32_t spp_handle = 0;

void bt_spp_cb(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
    static uint8_t rem_bda[6];

    switch (event)
    {
        case ESP_SPP_INIT_EVT:
            INFO("ESP_SPP_INIT_EVT");
            esp_spp_start_srv(ESP_SPP_SEC_NONE, ESP_SPP_ROLE_SLAVE, 0, "Strato SPP");
            break;

        case ESP_SPP_CLOSE_EVT:
            INFO("ESP_SPP_CLOSE_EVT");
            bt_notify(rem_bda, "", PROTO_BT_MODE_DISCONNECTED | PROTO_BT_MODE_SPP);
            spp_handle = 0;
            break;

        case ESP_SPP_DATA_IND_EVT:
        {
            INFO("ESP_SPP_DATA_IND_EVT len=%d handle=%d",
                     param->data_ind.len, param->data_ind.handle);

            debug_dump(param->data_ind.data, param->data_ind.len);

            proto_tele_recv_t data;
            data.len = param->data_ind.len;
            memcpy(data.message, param->data_ind.data,param->data_ind.len);
            protocol_send(PROTO_TELE_RECV, (void *)&data, sizeof(data));
        }
        break;

        case ESP_SPP_SRV_OPEN_EVT:
            INFO("ESP_SPP_SRV_OPEN_EVT");
            bt_notify(param->srv_open.rem_bda, "", PROTO_BT_MODE_CONNECTED | PROTO_BT_MODE_SPP);
            memcpy(rem_bda, param->srv_open.rem_bda, 6);
            spp_handle = param->srv_open.handle;
            break;

        default:
        break;
    }
}

void bt_spp_send(char * message, uint16_t len)
{
    if (spp_handle)
    {
        esp_spp_write(spp_handle, len, (uint8_t *)message);
    }
}

void bt_spp_init()
{
    esp_spp_init(ESP_SPP_MODE_CB);
    esp_spp_register_callback(bt_spp_cb);
}
