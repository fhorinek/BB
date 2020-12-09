/*
 * stream.c
 *
 *  Created on: Dec 3, 2020
 *      Author: horinek
 */

#include "stream.h"

void stream_init(stream_t * stream, uint8_t * buffer, uint16_t buffer_size, stream_handler_t handler)
{
    stream->buffer = buffer;
    stream->buffer_size = buffer_size;

    stream->lenght = 0;
    stream->state = stream_idle;

    stream->handler = handler;
}

void stream_packet(uint8_t * out, uint8_t * in, uint16_t in_size)
{
    uint16_t lenght = in_size + STREAM_OVERHEAD;

    uint8_t crc;
    out[0] = STREAM_STARTBYTE;
    out[1] = in_size & 0x00FF;
    out[2] = (in_size & 0xFF00) >> 8;

    crc = calc_crc(0x00, STREAM_CRC_KEY, out[1]);
    crc = calc_crc(crc, STREAM_CRC_KEY, out[2]);
    out[3] = crc;

    for (uint16_t i = 0; i < in_size; i++)
    {
        out[4 + i] = in[i];
        crc = calc_crc(crc, STREAM_CRC_KEY, in[i]);
    }

    out[lenght - 1] = crc;
}

void stream_parse(stream_t *stream, uint8_t data)
{
    switch (stream->state)
    {
        case (stream_idle):
            if (data == STREAM_STARTBYTE)
            {
                stream->state = stream_length_lo;

                if (stream->lenght > 0)
                {
                    stream->buffer[stream->lenght + 1] = 0;
                    stream->handler(stream->buffer, stream->lenght, stream_res_dirty);
                    stream->lenght = 0;
                }
            }
            else
            {
                stream->buffer[stream->lenght] = data;
                stream->lenght++;

                if (stream->lenght >= stream->buffer_size || data == '\n')
                {
                    stream->lenght--;
                    stream->buffer[stream->lenght] = 0;

                    stream->handler(stream->buffer, stream->lenght, stream_res_dirty);
                    stream->lenght = 0;
                }

                break;
            }
        break;

        case (stream_length_lo):
            stream->state = stream_length_hi;
            stream->lenght = data;
            stream->crc = calc_crc(0x00, STREAM_CRC_KEY, data);
        break;

        case (stream_length_hi):
            stream->lenght |= data << 8;
            if (stream->lenght > stream->buffer_size)
            {
                ERR("Stream message %u is larger than buffer size %u", stream->lenght, stream->buffer_size);
                stream->handler(stream->buffer, stream->lenght, stream_res_error);
                stream->state = stream_idle;
                stream->lenght = 0;
                break;
            }
            stream->index = 0;
            stream->crc = calc_crc(stream->crc, STREAM_CRC_KEY, data);
            stream->state = stream_head_crc;
        break;

        case (stream_head_crc):
            if (stream->crc == data)
            {
                stream->state = stream_data;
            }
            else
            {
                ERR("Stream incorrect head CRC %02X != %02X", stream->crc, data);
                stream->state = stream_idle;
                stream->lenght = 0;
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
        }
        break;

        case (stream_crc):
        {
            if (stream->crc == data)
            {
                stream->handler(stream->buffer, stream->lenght, stream_res_message);
            }
            else
            {
                ERR("Stream incorrect body CRC %02X != %02X", stream->crc, data);
                stream->handler(stream->buffer, stream->lenght, stream_res_error);
            }

            stream->state = stream_idle;
            stream->lenght = 0;
        }
        break;
    }
}
