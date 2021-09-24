/*
 * bluetooth.c
 *
 *  Created on: 22. 9. 2021
 *      Author: horinek
 */

#include "bluetooth.h"

#include "pipeline/bluetooth.h"

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_a2dp_api.h"
#include "esp_avrc_api.h"
#include "esp_spp_api.h"
#include "esp_hf_client_api.h"

#include "protocol.h"

static SemaphoreHandle_t bt_lock;

void bt_notify(uint8_t * mac, char * name, uint8_t mode)
{
    proto_bt_notify_t data;
    memcpy(data.dev, mac, 6);
    strncpy(data.dev_name, name, PROTO_BT_DEV_NAME_LEN - 1);
    data.mode = mode;
    protocol_send(PROTO_BT_NOTIFY, (void *)&data, sizeof(data));
}

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

void bt_avrc_cb(esp_avrc_ct_cb_event_t event, esp_avrc_ct_cb_param_t *param)
{
    INFO("AVRC %u", event);
}

void bt_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param)
{
    switch (event)
    {
        case ESP_BT_GAP_AUTH_CMPL_EVT:
        {
            if (param->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS)
            {
                INFO( "authentication success: %s", param->auth_cmpl.device_name);
                bt_notify(param->auth_cmpl.bda, (char *)param->auth_cmpl.device_name, PROTO_BT_MODE_PAIRED);
            }
            else
            {
                ERR("authentication failed, status:%d", param->auth_cmpl.bda);
                proto_bt_pair_req_t data;
                memcpy(data.dev, param->auth_cmpl.bda, 6);
                data.cancel = true;
                protocol_send(PROTO_BT_PAIR_REQ, (void *)&data, sizeof(data));
            }
            break;
        }

        case ESP_BT_GAP_CFM_REQ_EVT:
        {
            INFO( "ESP_BT_GAP_CFM_REQ_EVT Please compare the numeric value: %d", param->cfm_req.num_val);
            proto_bt_pair_req_t data;
            memcpy(data.dev, param->cfm_req.bda, 6);
            data.value = param->cfm_req.num_val;
            data.cancel = false;
            protocol_send(PROTO_BT_PAIR_REQ, (void *)&data, sizeof(data));
        }
        break;

        default:
        {
            INFO( "event: %d", event);
            break;
        }
    }
    return;
}

static void bt_spp_cb(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
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
            break;

        case ESP_SPP_DATA_IND_EVT:
            INFO("ESP_SPP_DATA_IND_EVT len=%d handle=%d",
                     param->data_ind.len, param->data_ind.handle);
            debug_dump(param->data_ind.data,param->data_ind.len);
            break;

        case ESP_SPP_SRV_OPEN_EVT:
            INFO("ESP_SPP_SRV_OPEN_EVT");
            bt_notify(param->srv_open.rem_bda, "", PROTO_BT_MODE_CONNECTED | PROTO_BT_MODE_SPP);
            memcpy(rem_bda, param->srv_open.rem_bda, 6);
            break;
        default:
            break;
    }
}



void bt_set_discoverable(proto_bt_discoverable_t * packet)
{
    esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, (packet->enabled) ? ESP_BT_GENERAL_DISCOVERABLE : ESP_BT_NON_DISCOVERABLE);
}

void bt_confirm_pair(proto_bt_pair_res_t * packet)
{
    esp_bt_gap_ssp_confirm_reply(packet->dev, packet->pair);
}

void bt_set_mode(proto_set_bt_mode_t *packet)
{
    xSemaphoreTake(bt_lock, WAIT_INF);

    esp_bt_mode_t req_bt_mode = ESP_BT_MODE_IDLE;

    if (packet->enabled)
    {
        if (packet->a2dp || packet->spp)
        {
            req_bt_mode = (packet->ble) ? ESP_BT_MODE_BTDM : ESP_BT_MODE_CLASSIC_BT;
        }
        else
        {
            if (packet->ble)
            {
                req_bt_mode = ESP_BT_MODE_BLE;
            }
        }
    }

    proto_bt_mode_t data;
    data.enabled = false;

    if (req_bt_mode != ESP_BT_MODE_IDLE)
    {
        //init bluetooth
        esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
        ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_BTDM));
        ESP_ERROR_CHECK(esp_bluedroid_init());
        ESP_ERROR_CHECK(esp_bluedroid_enable());

        //set name
        esp_bt_dev_set_device_name(packet->name);
        esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_NON_DISCOVERABLE);

        //enable SSP pairing
        esp_bt_sp_param_t param_type = ESP_BT_SP_IOCAP_MODE;
        esp_bt_io_cap_t iocap = ESP_BT_IO_CAP_IO;
        esp_bt_gap_set_security_param(param_type, &iocap, sizeof(uint8_t));

        //enable legacy pairing
        esp_bt_pin_type_t pin_type = ESP_BT_PIN_TYPE_VARIABLE;
        esp_bt_gap_set_pin(pin_type, strlen(packet->pin), (uint8_t *)packet->pin);

        //add callbacks
        esp_bt_gap_register_callback(bt_gap_cb);

        if (packet->a2dp)
        {
            pipe_bluetooth_init();

            //add callbacks
            esp_a2d_register_callback(bt_a2d_cb);
            esp_avrc_ct_register_callback(bt_avrc_cb);

            //esp_hf_client_register_callback(bt_hf_client_cb);
            esp_hf_client_init();
        }

        if (packet->spp)
        {
            esp_spp_init(ESP_SPP_MODE_CB);
            esp_spp_register_callback(bt_spp_cb);
        }

        data.enabled = true;
    }

    protocol_send(PROTO_BT_MODE, (void*) &data, sizeof(data));

    xSemaphoreGive(bt_lock);

    free(packet);
    vTaskDelete(NULL);
}

void bt_reset_devices()
{
    //TODO remove all paired/bonded devices
}

void bt_init()
{
    bt_lock = xSemaphoreCreateBinary();

    xSemaphoreGive(bt_lock);
}

