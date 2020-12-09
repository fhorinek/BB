/*
 * protocol.c
 *
 *  Created on: 4. 12. 2020
 *      Author: horinek
 */

#define DEBUG_LEVEL DGB_DEBUG

#include "protocol.h"
#include "drivers/uart.h"
#include "etc/stream.h"
#include "drivers/tas5720.h"

void protocol_send_version()
{
    DBG("sending version");
    proto_version_t data;
    data.version = 0x00ABCDEF;
    protocol_send(PROTO_VERSION, (uint8_t *)&data, sizeof(data));
}

void protocol_send(uint8_t type, uint8_t *data, uint16_t data_len)
{
    uint8_t buf_in[data_len + 1];
    uint8_t buf_out[data_len + 1 + STREAM_OVERHEAD];

    buf_in[0] = type;
    memcpy((void*) buf_in + 1, (void*) data, data_len);

    stream_packet(buf_out, buf_in, sizeof(buf_in));

    uart_send(buf_out, sizeof(buf_out));
}

void protocol_handle(uint8_t *data, uint16_t len)
{
    uint8_t type = data[0];
    data++;
    len--;

    switch (type)
    {
        case (PROTO_PING):
            DBG("sending pong");
            protocol_send(PROTO_PONG, NULL, 0);
        break;

        case (PROTO_GET_VERSION):
        {
            protocol_send_version();
        }
        break;

        case (PROTO_SET_VOLUME):
        {
            proto_volume_t * packet = data;

            if (packet->type == PROTO_VOLUME_MASTER)
                tas_volume(packet->val);
        }
        break;

        default:
            DBG("Unknown packet");
            DUMP(data, len);
        break;
    }
}
