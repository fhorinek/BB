/*
 * stream.c
 *
 *  Created on: Dec 3, 2020
 *      Author: horinek
 */

#include "../../main/etc/stream.h"

void stream_init(stream_t * stream, uint8_t * buffer, uint16_t buffer_size)
{
    stream->buffer = buffer;
    stream->buffer_size = buffer_size;

    stream->state = stream_idle;
}

void stream_reset(stream_t * stream)
{
    stream->state = stream_idle;
}

void stream_packet(uint8_t type, uint8_t * out, uint8_t * in, uint16_t in_size)
{
    uint16_t lenght = in_size + STREAM_OVERHEAD;

    uint8_t crc;
    out[0] = STREAM_STARTBYTE;
    out[1] = type;
    out[2] = in_size & 0x00FF;
    out[3] = (in_size & 0xFF00) >> 8;

    crc = calc_crc(0x00, STREAM_CRC_KEY, out[1]);
    crc = calc_crc(crc, STREAM_CRC_KEY, out[2]);
    crc = calc_crc(crc, STREAM_CRC_KEY, out[3]);
    out[4] = crc;

    for (uint16_t i = 0; i < in_size; i++)
    {
        out[5 + i] = in[i];
        crc = calc_crc(crc, STREAM_CRC_KEY, in[i]);
    }

    out[lenght - 1] = crc;
}

bool stream_parse(stream_t *stream, uint8_t data)
{
    switch (stream->state)
    {
        case (stream_idle):
            if (data == STREAM_STARTBYTE)
            {
                stream->state = stream_packet_type;
            }
        break;

        case (stream_packet_type):
            stream->state = stream_length_lo;
            stream->packet_type = data;
            stream->crc = calc_crc(0x00, STREAM_CRC_KEY, data);
        break;

        case (stream_length_lo):
            stream->state = stream_length_hi;
            stream->lenght = data;
            stream->crc = calc_crc(stream->crc, STREAM_CRC_KEY, data);
        break;

        case (stream_length_hi):
            stream->lenght |= data << 8;
            if (stream->lenght > stream->buffer_size)
            {
                ERR("Stream message %u is larger than buffer size %u", stream->lenght, stream->buffer_size);
                stream->state = stream_idle;
                break;
            }
            stream->index = 0;
            stream->crc = calc_crc(stream->crc, STREAM_CRC_KEY, data);
            stream->state = stream_head_crc;
        break;

        case (stream_head_crc):
            if (stream->crc == data)
            {
            	if (stream->lenght > 0)
            		stream->state = stream_data;
            	else
            		stream->state = stream_crc;
            }
            else
            {
                ERR("Stream incorrect head CRC %02X != %02X", stream->crc, data);
                stream->state = stream_idle;
            }
        break;

        case (stream_data):
        {
            stream->buffer[stream->index] = data;
            stream->crc = calc_crc(stream->crc, STREAM_CRC_KEY, data);
            stream->index++;
            if (stream->index >= stream->lenght)
            {
                stream->state = stream_crc;
            }
            break;
        }

        case (stream_crc):
        {
            stream->state = stream_idle;

            if (stream->crc == data)
            {
                return true;
            }
            else
            {
                ERR("Stream incorrect body CRC %02X != %02X", stream->crc, data);
            }
        }
    }

    return false;
}
