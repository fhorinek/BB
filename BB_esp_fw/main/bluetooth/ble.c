/*
 * ble.c
 *
 *  Created on: 4. 10. 2021
 *      Author: horinek
 */

#include "ble.h"

#include "esp_gatts_api.h"
#include "esp_gap_ble_api.h"
#include "bluetooth.h"
#include "esp_gattc_api.h"
#include "esp_gatt_common_api.h"

#include "protocol.h"

static esp_ble_adv_params_t ble_adv_params = {
        .adv_int_min = 0x20,
        .adv_int_max = 0x40,
        .adv_type = ADV_TYPE_IND,
        .own_addr_type = BLE_ADDR_TYPE_RPA_RANDOM,
//        .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
        .channel_map = ADV_CHNL_ALL,
        .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

enum
{
    SPP_IDX_SVC,

    SPP_IDX_SPP_DATA_CHAR,
    SPP_IDX_SPP_DATA_VAL,
    SPP_IDX_SPP_DATA_NTF_CFG,

    SPP_IDX_NB,
};

#define ESP_SPP_APP_ID          0x56
#define SPP_SVC_INST_ID         0

/// SPP Service
/// Characteristic UUID
#define ESP_GATT_UUID_SPP_DATA      0xFFE1

#define CHAR_DECLARATION_SIZE   (sizeof(uint8_t))

static const uint16_t primary_service_uuid = ESP_GATT_UUID_PRI_SERVICE;
static const uint16_t character_declaration_uuid = ESP_GATT_UUID_CHAR_DECLARE;
static const uint16_t character_client_config_uuid = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;

static const uint8_t char_prop_write_notify = ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_NOTIFY;

#define SPP_DATA_MAX_LEN           (512)
#define SPP_DATA_BUFF_MAX_LEN      (2*1024)

static const uint16_t spp_data_uuid = ESP_GATT_UUID_SPP_DATA;
static const uint8_t spp_data_val[20] = { 0x00 };
static const uint8_t spp_data_notify_ccc[2] = { 0x00, 0x00 };

//static const uint16_t spp_service_uuid = 0xABF0;

const uint8_t spp_service_uuid[] = {0x1b, 0xc5, 0xd5, 0xa5, 0x02, 0x00, 0x03, 0xa9, 0xe3, 0x11, 0x8b, 0xaa, 0xa0, 0xc6, 0x79, 0xe0};
//const uint8_t spp_over_ble_characteristic_uuid[] = {0x1b, 0xc5, 0xd5, 0xa5, 0x02, 0x00, 0xef, 0x9c, 0xe3, 0x11, 0x89, 0xaa, 0xc0, 0x12, 0x83, 0xb3};


static const esp_gatts_attr_db_t spp_gatt_db[SPP_IDX_NB] =
{
    //SPP -  Service Declaration
    [SPP_IDX_SVC] =
    {
        { ESP_GATT_AUTO_RSP },
        { ESP_UUID_LEN_16,
            (uint8_t*) &primary_service_uuid,
            ESP_GATT_PERM_READ,
            sizeof(spp_service_uuid),
            sizeof(spp_service_uuid),
            (uint8_t*) spp_service_uuid }
    },

    //SPP -  data characteristic Declaration
    [SPP_IDX_SPP_DATA_CHAR] =
    {
        { ESP_GATT_AUTO_RSP },
        { ESP_UUID_LEN_16,
            (uint8_t*) &character_declaration_uuid,
            ESP_GATT_PERM_READ,
            CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE,
            (uint8_t*) &char_prop_write_notify }
    },

    //SPP -  data characteristic Value
    [SPP_IDX_SPP_DATA_VAL] =
    {
        { ESP_GATT_AUTO_RSP },
        { ESP_UUID_LEN_16,
            (uint8_t*) &spp_data_uuid,
            ESP_GATT_PERM_WRITE,
            SPP_DATA_MAX_LEN, sizeof(spp_data_val),
            (uint8_t*) spp_data_val }
    },

    //SPP -  data notify characteristic Declaration
    [SPP_IDX_SPP_DATA_NTF_CFG] =
    {
        { ESP_GATT_AUTO_RSP },
        { ESP_UUID_LEN_16,
            (uint8_t*) &character_client_config_uuid,
            ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
            sizeof(spp_data_notify_ccc), sizeof(spp_data_notify_ccc),
            (uint8_t*) &spp_data_notify_ccc }
    },
};

static esp_gatt_if_t spp_gatts_if = 0xFF;
static bool spp_is_connected = false;

#define MTU_DEFAULT 23

static uint16_t spp_handle_table[SPP_IDX_NB];
static uint16_t spp_mtu_size = MTU_DEFAULT;
static uint16_t spp_conn_id = 0xffff;

static uint8_t find_char_and_desr_index(uint16_t handle)
{
    uint8_t error = 0xff;

    for(int i = 0; i < SPP_IDX_NB ; i++)
    {
        if(handle == spp_handle_table[i])
        {
            return i;
        }
    }

    return error;
}

static bool ble_enable_notify = false;


static char bt_name[PROTO_BT_NAME_LEN];

static void ble_gatts_profile_cb(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    esp_ble_gatts_cb_param_t *p_data = (esp_ble_gatts_cb_param_t*) param;
    uint8_t res = 0xff;

    switch (event)
    {
        case ESP_GATTS_REG_EVT:
        {
            INFO("%s %d", __func__, __LINE__);
            esp_ble_gap_set_device_name(bt_name);
            uint8_t mac[6];
            esp_read_mac(mac, ESP_MAC_ETH);
            esp_ble_gap_set_rand_addr(mac);
            esp_ble_gap_config_local_icon(ESP_BLE_APPEARANCE_OUTDOOR_SPORTS_LOCATION_AND_NAV);

            uint8_t adv_data_len = 3 + 2 + strlen(bt_name) + 3;
            uint8_t * adv_data = malloc(adv_data_len);

            uint8_t i = 0;
            //Flags
            adv_data[i++] = 0x02; //lenght
            adv_data[i++] = ESP_BLE_AD_TYPE_FLAG;
            adv_data[i++] = 0x06; //LE General Discoverable Mode | no BR/EDR Capable

            //Device name
            adv_data[i++] = 0x01 + strlen(bt_name) + 3; //lenght
            adv_data[i++] = ESP_BLE_AD_TYPE_NAME_CMPL;
            memcpy(adv_data + i, bt_name, strlen(bt_name));
            i += strlen(bt_name);
            memcpy(adv_data + i, " LE", 3);

            INFO("%s %d", __func__, __LINE__);
            esp_ble_gap_config_adv_data_raw(adv_data, adv_data_len);
            free(adv_data);

            INFO("%s %d", __func__, __LINE__);
            esp_ble_gatts_create_attr_tab(spp_gatt_db, gatts_if, SPP_IDX_NB, SPP_SVC_INST_ID);
        }
        break;
        case ESP_GATTS_READ_EVT:
            res = find_char_and_desr_index(p_data->read.handle);
//            if (res == SPP_IDX_SPP_STATUS_VAL)
//            {
//                //TODO:client read the status characteristic
//            }
        break;
        case ESP_GATTS_WRITE_EVT:
            {
            res = find_char_and_desr_index(p_data->write.handle);
            if (p_data->write.is_prep == false)
            {
                INFO("ESP_GATTS_WRITE_EVT : handle = %d", res);
                INFO("  p_data->write.len %u", p_data->write.len);
                debug_dump(p_data->write.value, p_data->write.len);

                if (res == SPP_IDX_SPP_DATA_NTF_CFG)
                {
                    if ((p_data->write.len == 2) && (p_data->write.value[0] == 0x01) && (p_data->write.value[1] == 0x00))
                    {
                        ble_enable_notify = true;
                    }
                    else if ((p_data->write.len == 2) && (p_data->write.value[0] == 0x00) && (p_data->write.value[1] == 0x00))
                    {
                        ble_enable_notify = false;
                    }
                }
                else if (res == SPP_IDX_SPP_DATA_VAL)
                {
                    INFO("BLE UART RX");

                    proto_tele_recv_t data;
                    data.len = p_data->write.len;
                    memcpy(data.message, p_data->write.value, p_data->write.len);
                    protocol_send(PROTO_TELE_RECV, (void *)&data, sizeof(data));
                }
            }
            break;
        }
        case ESP_GATTS_EXEC_WRITE_EVT:
        break;

        case ESP_GATTS_MTU_EVT:
            spp_mtu_size = p_data->mtu.mtu;
            INFO("MTU changed to %u", spp_mtu_size);
        break;
        case ESP_GATTS_CONNECT_EVT:
            spp_mtu_size = MTU_DEFAULT;
            spp_conn_id = p_data->connect.conn_id;
            spp_gatts_if = gatts_if;
            spp_is_connected = true;
            esp_ble_set_encryption(param->connect.remote_bda, ESP_BLE_SEC_ENCRYPT_MITM);
            bt_notify(p_data->connect.remote_bda, "", PROTO_BT_MODE_BLE | PROTO_BT_MODE_CONNECTED);
        break;
        case ESP_GATTS_DISCONNECT_EVT:
            spp_is_connected = false;
            ble_enable_notify = false;
            bt_notify(p_data->connect.remote_bda, "", PROTO_BT_MODE_BLE | PROTO_BT_MODE_DISCONNECTED);
            esp_ble_gap_start_advertising(&ble_adv_params);
        break;
        case ESP_GATTS_CREAT_ATTR_TAB_EVT:
            {
            INFO("The number handle = %x", param->add_attr_tab.num_handle);
            if (param->add_attr_tab.status != ESP_GATT_OK)
            {
                ERR("Create attribute table failed, error code=0x%x", param->add_attr_tab.status);
            }
            else if (param->add_attr_tab.num_handle != SPP_IDX_NB)
            {
                ERR("Create attribute table abnormally, num_handle (%d) doesn't equal to HRS_IDX_NB(%d)", param->add_attr_tab.num_handle, SPP_IDX_NB);
            }
            else
            {
                memcpy(spp_handle_table, param->add_attr_tab.handles, sizeof(spp_handle_table));
                esp_ble_gatts_start_service(spp_handle_table[SPP_IDX_SVC]);
            }
            break;
        }
        case ESP_GATTS_CONF_EVT:
        break;

        default:
            INFO("ble_gatts_profile_cb event = %u", event);
        break;
    }
}

static void ble_gatts_cb(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    /* If event is register event, store the gatts_if for each profile */
    if (event == ESP_GATTS_REG_EVT)
    {
        if (param->reg.status == ESP_GATT_OK)
        {
            spp_gatts_if = gatts_if;
        }
        else
        {
            INFO("Reg app failed, app_id %04x, status %d", param->reg.app_id, param->reg.status);
            return;
        }
    }

    if (gatts_if == ESP_GATT_IF_NONE
            || gatts_if == spp_gatts_if)
    {
        ble_gatts_profile_cb(event, gatts_if, param);
    }
}

void ble_spp_send(char * data, uint16_t len)
{
    if (spp_is_connected && ble_enable_notify)
    {
        uint16_t index = 0;
        while (index < len)
        {
            uint16_t chunk = min(len - index, spp_mtu_size - 3);

            esp_ble_gatts_send_indicate(spp_gatts_if, spp_conn_id, spp_handle_table[SPP_IDX_SPP_DATA_VAL],
                    chunk, (uint8_t *)(data + index), false);

            index += chunk;
        }
    }
}

void ble_unpair_all()
{
    int dev_num = esp_ble_get_bond_device_num();

    esp_ble_bond_dev_t *dev_list = (esp_ble_bond_dev_t *)malloc(sizeof(esp_ble_bond_dev_t) * dev_num);
    esp_ble_get_bond_device_list(&dev_num, dev_list);
    for (uint16_t i = 0; i < dev_num; i++)
    {
        esp_ble_remove_bond_device(dev_list[i].bd_addr);
    }

    free(dev_list);
}

static proto_mac_t paired_mac = {0};

void ble_confirm(proto_mac_t dev, bool pair)
{
    esp_ble_confirm_reply(dev, pair);

    if (pair)
        memcpy(paired_mac, dev, 6);
    else
        memset(paired_mac, 0, 6);
}

static void ble_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    esp_err_t err;

    switch (event)
    {
        case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:
            esp_ble_gap_start_advertising(&ble_adv_params);
        break;

        case ESP_GAP_BLE_NC_REQ_EVT:
            INFO( "ESP_GAP_BLE_NC_REQ_EVT Please compare the numeric value: %d", param->ble_security.key_notif.passkey);
            proto_bt_pair_req_t data;
            memcpy(data.dev, param->ble_security.ble_req.bd_addr, 6);
            data.value = param->ble_security.key_notif.passkey;
            data.cancel = false;
            data.ble = true;
            protocol_send(PROTO_BT_PAIR_REQ, (void *)&data, sizeof(data));
        break;

        case ESP_GAP_BLE_AUTH_CMPL_EVT:
        {
            if (param->ble_security.auth_cmpl.success)
            {
                INFO("authentication success");
                if (memcmp(param->ble_security.auth_cmpl.bd_addr, paired_mac, 6) == 0)
                {
                    memset(paired_mac, 0, 6);
                    bt_notify(param->ble_security.auth_cmpl.bd_addr, "", PROTO_BT_MODE_PAIRED);
                }
            }
            else
            {
                ERR("authentication failed, status:%d", param->ble_security.auth_cmpl.auth_mode);
                proto_bt_pair_req_t data;
                memcpy(data.dev, param->ble_security.auth_cmpl.bd_addr, 6);
                data.cancel = true;
                data.ble = true;
                protocol_send(PROTO_BT_PAIR_REQ, (void *)&data, sizeof(data));
            }
            break;
        }


        case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
            //advertising start complete event to indicate advertising start successfully or failed
            if ((err = param->adv_start_cmpl.status) != ESP_BT_STATUS_SUCCESS)
            {
                ERR("Advertising start failed: %s", esp_err_to_name(err));
            }
        break;
        default:
            INFO("ble_gap_cb event = %u", event);
        break;
    }
}

void ble_init(char * name)
{
    strcpy(bt_name, name);
    esp_ble_gatts_register_callback(ble_gatts_cb);
    esp_ble_gap_register_callback(ble_gap_cb);
    esp_ble_gatts_app_register(ESP_SPP_APP_ID);

    esp_ble_auth_req_t auth_req = ESP_LE_AUTH_REQ_SC_MITM_BOND;
    esp_ble_gap_set_security_param(ESP_BLE_SM_AUTHEN_REQ_MODE, &auth_req, sizeof(uint8_t));

    uint8_t iocap = ESP_IO_CAP_IO;
    esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &iocap, sizeof(uint8_t));

    uint8_t key_size = 8;      //the key size should be 7~16 bytes
    esp_ble_gap_set_security_param(ESP_BLE_SM_MAX_KEY_SIZE, &key_size, sizeof(uint8_t));

    uint8_t init_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK | ESP_BLE_CSR_KEY_MASK | ESP_BLE_LINK_KEY_MASK;
    uint8_t rsp_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK | ESP_BLE_CSR_KEY_MASK | ESP_BLE_LINK_KEY_MASK;
    esp_ble_gap_set_security_param(ESP_BLE_SM_SET_INIT_KEY, &init_key, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_SET_RSP_KEY, &rsp_key, sizeof(uint8_t));
//
//    uint32_t passkey = 123456;
//    esp_ble_gap_set_security_param(ESP_BLE_SM_SET_STATIC_PASSKEY, &passkey, sizeof(uint32_t));

    esp_ble_gatt_set_local_mtu(128);
}

