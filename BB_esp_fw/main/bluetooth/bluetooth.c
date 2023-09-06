/*
 * bluetooth.c
 *
 *  Created on: 22. 9. 2021
 *      Author: horinek
 */

#include "bluetooth.h"


#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_gap_ble_api.h"

#include "a2dp.h"
#include "spp.h"
#include "ble.h"

#include "protocol.h"

static SemaphoreHandle_t bt_lock;

static char bt_pin[PROTO_BT_PIN_LEN];

void bt_notify(uint8_t * mac, char * name, uint8_t mode)
{
    proto_bt_notify_t data;
    memcpy(data.dev, mac, 6);
    strncpy(data.dev_name, name, PROTO_BT_DEV_NAME_LEN - 1);
    data.mode = mode;
    protocol_send(PROTO_BT_NOTIFY, (void *)&data, sizeof(data));
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
                ERR("authentication failed, status:%d", param->auth_cmpl.stat);
                proto_bt_pair_req_t data;
                memcpy(data.dev, param->auth_cmpl.bda, 6);
                data.cancel = true;
                data.ble = true;
                data.only_show = false;
                protocol_send(PROTO_BT_PAIR_REQ, (void *)&data, sizeof(data));
            }
            break;
        }

        case ESP_BT_GAP_PIN_REQ_EVT:
            INFO("ESP_BT_GAP_PIN_REQ_EVT min_16_digit:%d", param->pin_req.min_16_digit);
            esp_bt_gap_pin_reply(param->pin_req.bda, true, strlen(bt_pin), (uint8_t *)bt_pin);
        break;

        case ESP_BT_GAP_CFM_REQ_EVT:
        {
            INFO( "ESP_BT_GAP_CFM_REQ_EVT Please compare the numeric value: %d", param->cfm_req.num_val);
            proto_bt_pair_req_t data;
            memcpy(data.dev, param->cfm_req.bda, 6);
            data.value = param->cfm_req.num_val;
            data.cancel = false;
            data.ble = false;
            data.only_show = false;
            protocol_send(PROTO_BT_PAIR_REQ, (void *)&data, sizeof(data));
        }
        break;

        case ESP_BT_GAP_KEY_NOTIF_EVT:
        {
            INFO( "ESP_BT_GAP_KEY_NOTIF_EVT  Please enter the numeric value on second device: %d", param->key_notif.passkey);
            proto_bt_pair_req_t data;
            memcpy(data.dev, param->key_notif.bda, 6);
            data.value = param->key_notif.passkey;
            data.cancel = false;
            data.ble = false;
            data.only_show = true;
            protocol_send(PROTO_BT_PAIR_REQ, (void *)&data, sizeof(data));
        }
        break;

        default:
        {
            INFO( "bt_gap_cb event: %d", event);
            break;
        }
    }
    return;
}

void bt_unapir_all(void)
{
    int dev_num = esp_bt_gap_get_bond_device_num();

    esp_bd_addr_t * dev_list = (esp_bd_addr_t *)malloc(sizeof(esp_bd_addr_t) * dev_num);
    esp_bt_gap_get_bond_device_list(&dev_num, dev_list);
    for (uint16_t i = 0; i < dev_num; i++)
    {
        esp_bt_gap_remove_bond_device (dev_list[i]);
    }

    free(dev_list);
}


void bt_unpair()
{
	xSemaphoreTake(bt_lock, WAIT_INF);
    bt_unapir_all();
    ble_unpair_all();
    xSemaphoreGive(bt_lock);
}

void bt_tele_send(proto_tele_send_t * packet)
{
	xSemaphoreTake(bt_lock, WAIT_INF);
    bt_spp_send(packet->message, packet->len);
    ble_spp_send(packet->message, packet->len);

    protocol_send(PROTO_TELE_SEND_ACK, NULL, 0);
    xSemaphoreGive(bt_lock);
}

static esp_bt_mode_t req_bt_mode = ESP_BT_MODE_IDLE;

void bt_set_discoverable(proto_bt_discoverable_t * packet)
{
	xSemaphoreTake(bt_lock, WAIT_INF);

	INFO("bt_set_discoverable");

    if (req_bt_mode & ESP_BT_MODE_CLASSIC_BT)
    {
    	ESP_ERROR_CHECK(esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, (packet->enabled) ? ESP_BT_GENERAL_DISCOVERABLE : ESP_BT_NON_DISCOVERABLE));
    }
    xSemaphoreGive(bt_lock);
}

void bt_confirm_pair(proto_bt_pair_res_t * packet)
{
	xSemaphoreTake(bt_lock, WAIT_INF);

	INFO("bt_confirm_pair");

    if (packet->ble)
    {
        ble_confirm(packet->dev, packet->pair);
    }
    else
    {
        esp_bt_gap_ssp_confirm_reply(packet->dev, packet->pair);
    }
    xSemaphoreGive(bt_lock);
}

void bt_set_mode(proto_set_bt_mode_t *packet)
{
    xSemaphoreTake(bt_lock, WAIT_INF);

    INFO("bt_set_mode");

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
        bt_cfg.mode = req_bt_mode;
        ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
        ESP_ERROR_CHECK(esp_bt_controller_enable(req_bt_mode));
        ESP_ERROR_CHECK(esp_bluedroid_init());
        ESP_ERROR_CHECK(esp_bluedroid_enable());


        if (packet->a2dp || packet->spp)
        {
            //set name
            esp_bt_dev_set_device_name(packet->name);
            esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_NON_DISCOVERABLE);

            //enable SSP pairing
            esp_bt_sp_param_t param_type = ESP_BT_SP_IOCAP_MODE;
            esp_bt_io_cap_t iocap = ESP_BT_IO_CAP_IO;
            esp_bt_gap_set_security_param(param_type, &iocap, sizeof(esp_bt_io_cap_t));

            //enable legacy pairing
            esp_bt_pin_type_t pin_type = ESP_BT_PIN_TYPE_VARIABLE;
            esp_bt_gap_set_pin(pin_type, strlen(packet->pin), (uint8_t *)packet->pin);
            strcpy(bt_pin, packet->pin);

            //add callbacks
            esp_bt_gap_register_callback(bt_gap_cb);
        }

        if (packet->a2dp)
        {
            bt_a2dp_init(packet->a2dp_autoconnect);
        }

        if (packet->spp)
        {
            bt_spp_init();
        }

        if (packet->ble)
        {
            ble_init(packet->name);
        }

        data.enabled = true;
    }

    protocol_send(PROTO_BT_MODE, (void*) &data, sizeof(data));

    xSemaphoreGive(bt_lock);

    free(packet);
    vTaskDelete(NULL);
}

void bt_init()
{
    bt_lock = xSemaphoreCreateBinary();

    xSemaphoreGive(bt_lock);
}

