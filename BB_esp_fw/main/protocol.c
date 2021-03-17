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

#include "wifi.h"

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

    DBG("protocol_handle %u", type);

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
            proto_volume_t *packet = data;

            if (packet->type == PROTO_VOLUME_MASTER)
                tas_volume(packet->val);
        }
        break;

        case (PROTO_SPI_PREPARE):
        {
            spi_prepare_buffer();
        }
        break;

        case (PROTO_SOUND_START):
		{
        	proto_sound_start_t * packet = data;
        	pipe_sound_start(packet->file_id, packet->file_type, packet->file_lenght);
		}
        break;

        case (PROTO_SOUND_STOP):
        	pipe_sound_stop();
        break;

        case (PROTO_WIFI_SET_MODE):
		{
        	proto_wifi_mode_t * packet = (proto_wifi_mode_t *)data;
        	wifi_enable(packet->client, packet->ap);
		}
        break;

        case (PROTO_SET_DEVICE_NAME):
        {
        	proto_set_device_name_t * packet = (proto_set_device_name_t * )data;
        	strncpy(config.device_name, packet->name, sizeof(config.device_name));
        }
        break;

        default:
            DBG("Unknown packet");
            DUMP(data, len);
        break;
    }
}
