/*
 * sound.c
 *
 *  Created on: Jan 20, 2021
 *      Author: horinek
 */

#define DEBUG_LEVEL     DBG_DEBUG

#include "sound.h"
#include "fatfs.h"
#include "gui/gui.h"

#include "drivers/esp/protocol.h"

FIL audio_file;
uint8_t audio_file_id = 0;
static bool audio_file_opened = false;

void sound_start(char * filename)
{
    if (audio_file_opened)
    {
        sound_close();
    }

    uint8_t ret = f_open(&audio_file, filename, FA_READ);
    if (ret == FR_OK)
    {
        //id 0 is invalid
        audio_file_id++;
        if (audio_file_id == 0)
            audio_file_id = 1;

        esp_sound_start(audio_file_id, PROTO_FILE_WAV, f_size(&audio_file));
        audio_file_opened = true;
    }
}

void sound_read_next(uint8_t id, uint32_t requested_size)
{
    DBG("sound_read_next %u %lu", id, requested_size);

    //acquire buffer
    uint16_t free_space;
    uint8_t * buf = esp_spi_acquire_buffer_ptr(&free_space);

    if (free_space < requested_size + sizeof(proto_spi_header_t))
        requested_size = free_space - sizeof(proto_spi_header_t);

    UINT br;
    uint8_t ret = f_read(&audio_file, buf + sizeof(proto_spi_header_t), requested_size, &br);
    if (ret == FR_OK)
    {
        if (f_eof(&audio_file))
        {
            sound_close();
        }
    }
    else
    {
        //only when error
        WARN("audio file not open for reading");
    }

    if (br > 0)
    {
        //add header
        proto_spi_header_t hdr;
        hdr.packet_type = SPI_EP_SOUND;
        hdr.data_id = id;
        hdr.data_len = br;
        memcpy(buf, &hdr,  + sizeof(proto_spi_header_t));

        //release buffer
        esp_spi_release_buffer(br + sizeof(proto_spi_header_t));
        esp_spi_prepare();
    }
    else
    {
        esp_spi_release_buffer(0);
    }
}

void sound_close()
{
    f_close(&audio_file);
    audio_file_opened = false;
}

void sound_stop()
{
    sound_close();
    esp_sound_stop();
}
