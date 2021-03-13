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
        }
        break;

        case(PROTO_SPI_READY):
        {
            spi_start_transfer(((proto_spi_ready_t *)data)->data_lenght);
        }
        break;

        case(PROTO_SOUND_REQ_MORE):
        {
            proto_sound_req_more_t * packet = data;
            sound_read_next(packet->id, packet->data_lenght);
        }
        break;

        default:
            DBG("Unknown packet: %02X", type);
            DUMP(data, len);
    }
}

