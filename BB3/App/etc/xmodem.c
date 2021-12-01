/*
 * xmodem.c
 *
 *  Created on: May 24, 2017
 *      Author: sid
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "../../../common.h"
#include "../../../misc/statuscode.h"
#include "../serial_posix.h"

#include "../../../print.h"
#define DEBUG_LVL 1

#define X_STX 0x02
#define X_ACK 0x06
#define X_NAK 0x15
#define X_EOF 0x04

struct xmodem_chunk
{
    uint8_t start;
    uint8_t block;
    uint8_t block_neg;
    uint8_t payload[1024];
    uint16_t crc;
}__attribute__((packed));

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

int xmodem_transmit(serial_t *serial, const char *filename)
{
    if(serial == NULL)
        return -1;

    int serial_fd = serial->fd;

    int ret;
    uint8_t answer;
    struct stat stat;

    uint8_t eof = X_EOF;
    struct xmodem_chunk chunk;

    int fd = open(filename, O_RDONLY);
    if (fd < 0)
        return -2;

    fstat(fd, &stat);
    size_t len = stat.st_size;
    const size_t map_len = len;
    const uint8_t *map_buf = mmap(NULL, map_len, PROT_READ, MAP_PRIVATE, fd, 0);
    const uint8_t *buf = map_buf;
    if (!buf)
    {
        close(fd);
        return EALLOC;
    }

    print_debug(5, "Waiting for receiver ping ...\n");
    int loop = 6;
    do
    {
        ret = read(serial_fd, &answer, sizeof(answer));
        if (ret != sizeof(answer) && --loop <= 0)
        {
            munmap((void*)map_buf, map_len);
            close(fd);
            return -3;
        }
    } while (answer != 'C');

    print_debug(3, "Sending %s ", filename);
    chunk.block = 1;
    chunk.start = X_STX;

    int sent_blocks=0;
    int failures = 0;
    while (len)
    {
        size_t z = 0;
        int next = 0;

        z = min(len, sizeof(chunk.payload));
        memcpy(chunk.payload, buf, z);
        memset(chunk.payload + z, 0xff, sizeof(chunk.payload) - z);

        chunk.crc = swap16(crc16(chunk.payload, sizeof(chunk.payload)));
        chunk.block_neg = 0xff - chunk.block;

        ret = write(serial_fd, &chunk, sizeof(chunk));
        if (ret != sizeof(chunk))
        {
            munmap((void*)map_buf, map_len);
            close(fd);
            return -5;
        }

        ret = read(serial_fd, &answer, sizeof(answer));
        if (ret != sizeof(answer))
        {
            munmap((void*)map_buf, map_len);
            close(fd);
            return -6;
        }
        switch (answer)
        {
            case X_NAK:
                failures++;
                break;
            case X_ACK:
                next = 1;
                sent_blocks++;
                break;
            default:
                failures++;
                break;
        }

        if (next)
        {
            chunk.block++;
            len -= z;
            buf += z;
        }

        if(failures > 100)
            break;
    }

    ret = write(serial_fd, &eof, sizeof(eof));
    if (ret != sizeof(eof))
    {
        munmap((void*)map_buf, map_len);
        close(fd);
        return -7;
    }

    munmap((void*)map_buf, map_len);
    close(fd);

    print_debug(1, "Xmodem done. Failure %d. Blocks %d\n", failures, sent_blocks);

    return failures>100?-2:0;
}
xmodem.c
Zobrazuje sa položka xmodem.c
