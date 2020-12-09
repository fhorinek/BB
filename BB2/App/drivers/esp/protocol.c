/*
 * protocol.c
 *
 *  Created on: Dec 4, 2020
 *      Author: horinek
 */

#define DEBUG_LEVEL DBG_DEBUG

#include "protocol.h"
#include "../etc/stream.h"
#include "../fc/fc.h"

#include "protocol_def.h"

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

void protocol_send(uint8_t type, uint8_t * data, uint16_t data_len)
{
    uint8_t buf_in[data_len + 1];
    uint8_t buf_out[data_len + 1 + STREAM_OVERHEAD];

    buf_in[0] = type;
    memcpy((void *)buf_in + 1, (void *)data, data_len);

    stream_packet(buf_out, buf_in, sizeof(buf_in));

    //TODO: DMA || IRQ?
    HAL_UART_Transmit(&esp_uart, buf_out, sizeof(buf_out), 100);
}

void protocol_handle(uint8_t * data, uint16_t len)
{
    uint8_t type = data[0];

    data++;
    len--;

    switch(type)
    {
        case(PROTO_DEBUG): //Debug output
        {
            uint8_t level = data[0];
            if (level == 0xFF)
                level = DBG_INFO;

            data[len] = 0;

            char buff[len + 4];
            sprintf(buff, " << %s", data + 1);

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

        default:
            DBG("Unknown packet");
            DUMP(data, len);
    }
}

