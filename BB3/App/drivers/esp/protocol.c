/*
 * protocol.c
 *
 *  Created on: Dec 4, 2020
 *      Author: horinek
 */

#define DEBUG_LEVEL DBG_DEBUG

#include "protocol.h"
#include "etc/stream.h"
#include "fc/fc.h"
#include "config/config.h"


void esp_send_ping()
{
    protocol_send(PROTO_PING, NULL, 0);
}

void esp_get_version()
{
    protocol_send(PROTO_PING, NULL, 0);
}

void esp_set_volume(uint8_t vol)
{
    proto_volume_t data;
    data.type = PROTO_VOLUME_MASTER;
    data.val = vol;

    protocol_send(PROTO_SET_VOLUME, (void *) &data, sizeof(data));
}

void esp_spi_prepare()
{
    protocol_send(PROTO_SPI_PREPARE, NULL, 0);
}

void esp_sound_start(uint8_t id, uint8_t type, uint32_t size)
{
    proto_sound_start_t data;
    data.file_id = id;
    data.file_type = type;
    data.file_lenght = size;

    protocol_send(PROTO_SOUND_START, (void *) &data, sizeof(data));
}

void esp_sound_stop()
{
    protocol_send(PROTO_SOUND_STOP, NULL, 0);
}

void esp_set_wifi_mode()
{
    proto_wifi_mode_t data;
    data.client = config_get_bool(&config.wifi.enabled) ? PROTO_WIFI_MODE_ON : PROTO_WIFI_MODE_OFF;
    data.ap = config_get_bool(&config.wifi.ap) ? PROTO_WIFI_MODE_ON : PROTO_WIFI_MODE_OFF;

    protocol_send(PROTO_WIFI_SET_MODE, (void *) &data, sizeof(data));
}

void esp_set_device_name()
{
    proto_set_device_name_t data;
    strncpy(data.name, config_get_text(&config.device_name), DEV_NAME_LEN);

    protocol_send(PROTO_SET_DEVICE_NAME, (void *) &data, sizeof(data));
}

void esp_wifi_start_scan(wifi_list_update_cb cb)
{
    fc.esp.wifi_list_cb = cb;

    protocol_send(PROTO_WIFI_SCAN_START, NULL, 0);
}

void esp_wifi_stop_scan()
{
    protocol_send(PROTO_WIFI_SCAN_STOP, NULL, 0);
}

void esp_set_bt_mode()
{

}

void esp_configure()
{
    //set volume
    esp_set_volume(config_get_int(&config.bluetooth.volume));

    //set device name
    esp_set_device_name();

    //set wifi mode
    esp_set_wifi_mode();

    //set bt mode
    esp_set_bt_mode();
}

void protocol_send(uint8_t type, uint8_t * data, uint16_t data_len)
{
    uint8_t buf_out[data_len + STREAM_OVERHEAD];

    stream_packet(type, buf_out, data, data_len);

    //TODO: DMA || IRQ?
    HAL_UART_Transmit(esp_uart, buf_out, sizeof(buf_out), 100);
}

void protocol_handle(uint8_t type, uint8_t * data, uint16_t len)
{
    if (type != PROTO_DEBUG)
        DBG("protocol_handle %u", type);

    switch(type)
    {
        case(PROTO_DEBUG): //Debug output
        {
            uint8_t level = data[0];
            if (level == 0xFF)
                level = DBG_INFO;

            data[len] = 0;

            char buff[len + 4];
            sprintf(buff, "\t\t\t\t\t%s", data + 1);

            debug_send(level, (char *)buff);
        }
        break;

        case(PROTO_PONG):
        {
            DBG("Pong rx");
        }
        break;

        case(PROTO_VERSION):
        {
            fc.esp.version = ((proto_version_t *)data)->version;
            fc.esp.mode = esp_normal;
            DBG("ESP fw: %08X", fc.esp.version);

            esp_configure();
        }
        break;

        case(PROTO_SPI_READY):
        {
            spi_start_transfer(((proto_spi_ready_t *)data)->data_lenght);
        }
        break;

        case(PROTO_SOUND_REQ_MORE):
        {
            proto_sound_req_more_t * packet = (proto_sound_req_more_t *)data;
            sound_read_next(packet->id, packet->data_lenght);
        }
        break;

        default:
            DBG("Unknown packet: %02X", type);
            DUMP(data, len);
    }
}

