/*
 * file_buffer.c
 *
 *  Created on: Nov 10, 2022
 *      Author: horinek
 */
#define DEBUG_LEVEL DBG_DEBUG
#include "file_buffer.h"

void file_buffer_init(file_buffer_t * fb, uint8_t * buffer, uint32_t buffer_size)
{
    fb->handle = 0;
    fb->buffer_size = buffer_size;
    fb->buffer = buffer;
}

bool file_buffer_is_open(file_buffer_t * fb)
{
    return fb->handle != 0;
}

bool file_buffer_open(file_buffer_t * fb, char * path)
{
    if (fb->handle != 0)
    {
        red_close(fb->handle);
    }

    fb->handle = red_open(path, RED_O_RDONLY);

    if (fb->handle < 0)
    {
        fb->handle = 0;

        return false;
    }

    fb->file_size = file_size(fb->handle);
    fb->chunk_size = min(fb->buffer_size, fb->file_size);
    fb->start_address = 0;

    int32_t rd = red_read(fb->handle, fb->buffer, fb->chunk_size);
    FASSERT(rd == (int32_t)fb->chunk_size);

    return true;
}

uint8_t * file_buffer_seek(file_buffer_t * fb, uint32_t address, uint32_t lenght)
{
    FASSERT(address + lenght <= fb->file_size);

    if (address + lenght > fb->start_address + fb->chunk_size || address < fb->start_address)
    {
        if (fb->file_size - address < fb->chunk_size)
        {
            address = fb->file_size - fb->chunk_size;
        }

        DBG("[fb_seek %08X]", address);
        red_lseek(fb->handle, address, RED_SEEK_SET);
        int32_t rd = red_read(fb->handle, fb->buffer, fb->chunk_size);
        FASSERT(rd == (int32_t)fb->chunk_size);

        fb->start_address = address;
    }

    return fb->buffer + (address - fb->start_address);
}

void file_buffer_close(file_buffer_t * fb)
{
    red_close(fb->handle);
    fb->handle = 0;
}
