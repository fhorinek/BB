/*
 * fanet_update.c
 *
 *  Created on: Nov 25, 2021
 *      Author: horinek
 */

#include "fanet_update.h"

#include "drivers/gnss/fanet.h"
#include "gui/statusbar.h"

#define X_STX 0x02
#define X_ACK 0x06
#define X_NAK 0x15
#define X_EOF 0x04

#define CRC_POLY 0x1021

static uint16_t crc_update(uint16_t crc_in, int incr)
{
    uint16_t xor = crc_in >> 15;
    uint16_t out = crc_in << 1;

    if (incr) out++;

    if (xor) out ^= CRC_POLY;

    return out;
}

static uint16_t crc16(const uint8_t *data, uint16_t size)
{
    uint16_t crc, i;

    for (crc = 0; size > 0; size--, data++)
        for (i = 0x80; i; i >>= 1)
            crc = crc_update(crc, *data & i);

    for (i = 0; i < 16; i++)
        crc = crc_update(crc, 0);

    return crc;
}
static uint16_t swap16(uint16_t in)
{
    return (in >> 8) | ((in & 0xff) << 8);
}
struct xmodem_chunk
{
    uint8_t _pad; //<< pad to align properly
    uint8_t start;
    uint8_t block;
    uint8_t block_neg;
    uint8_t payload[1024];
    uint16_t crc;
}__attribute__((packed));

#define FAIL_LIMIT  10

bool fanet_update_firmware()
{
    INFO("FANET module firmware update start");
    int32_t f = red_open(PATH_FANET_FW, RED_O_RDONLY);

    if (f > 0)
    {
        uint32_t len = file_size(f);
        uint32_t readed = 0;
        __align struct xmodem_chunk chunk;

        chunk.block = 1;
        chunk.start = X_STX;

        uint8_t failures = 0;
        bool read_next = true;
        uint16_t total_chunks = len / sizeof(chunk.payload) + (len % sizeof(chunk.payload) > 1);
        lv_obj_t * msg = statusbar_msg_add(STATUSBAR_MSG_PROGRESS, "Updating FANET");

//        //flush
//        while(fanet_get_waiting())
//        {
//            fanet_read_byte();
//        }

        while (len)
        {
            uint32_t to_send = min(len, sizeof(chunk.payload));

            //read next block
            if (read_next)
            {
                int32_t br = red_read(f, chunk.payload, to_send);
                if (br != to_send)
                {
                    ERR("Failed to read from file %d != %u", br, to_send);
                    statusbar_msg_close(msg);
                    statusbar_msg_add(STATUSBAR_MSG_ERROR, "FANET update failed!");

                    red_close(f);
                    return false;
                }
                readed += br;
                memset(chunk.payload + to_send, 0xFF, sizeof(chunk.payload) - to_send);

                chunk.crc = swap16(crc16(chunk.payload, sizeof(chunk.payload)));
                chunk.block_neg = 0xFF - chunk.block;

                read_next = false;
            }

            INFO("Updating block %u (%u failures so far)", chunk.block, failures);

            if (chunk.block % 2 == 0)
                statusbar_msg_update_progress(msg, (chunk.block * 100) / total_chunks);

            //transmit, beware of padding!!
            fanet_transmit((uint8_t *)(&chunk) + 1, sizeof(chunk) - 1);

            //wait for answer
            uint8_t i = 0;
            bool timeout = false;
            while(true)
            {
                if (fanet_get_waiting() > 0)
                {
                    //skip if read 'C' (old data, should be flushed)
                    if (fanet_peak_byte() == 'C')
                    {
                        WARN("Next byte is 'C' ");
                        fanet_read_byte();
                    }
                    else
                        break;
                }

                osDelay(1);
                i++;
                if (i > 200)
                {
                    WARN("No answer from module, timeout");

                    timeout = true;
                    break;
                }
            }


            uint8_t c = (timeout) ? 0xFF : fanet_read_byte();

            INFO("ANS C = %02X %c", c, c);

            switch(c)
            {
                case X_NAK:
                    failures++;
                    break;
                case X_ACK:
                    read_next = true;
                    chunk.block++;
                    len -= to_send;
                    break;
                default:
                    failures++;
                    break;
            }

            if(failures > FAIL_LIMIT)
            {
                break;
            }

        }

        red_close(f);

        uint8_t eof = X_EOF;
        HAL_UART_Transmit(fanet_uart, &eof, sizeof(eof), 100);

        statusbar_msg_close(msg);

        if (failures > FAIL_LIMIT)
        {
            statusbar_msg_add(STATUSBAR_MSG_ERROR, "FANET update failed!");
            return false;
        }

        statusbar_msg_add(STATUSBAR_MSG_INFO, "FANET updated!");
        INFO("Update done");
        return true;
    }

    ERR("Unable to open firmware file");
    return false;
}
