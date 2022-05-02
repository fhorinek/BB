/*
 * sound.c
 *
 *  Created on: Jan 20, 2021
 *      Author: horinek
 */

#define DEBUG_LEVEL     DBG_DEBUG

#include "sound.h"
#include "gui/gui.h"

#include "drivers/esp/protocol.h"

int32_t audio_file;
uint8_t audio_file_id = 0;
static bool audio_file_opened = false;

void sound_start(char * filename)
{
    if (audio_file_opened)
    {
        sound_close();
    }

    audio_file = red_open(filename, RED_O_RDONLY);
    if (audio_file > 0)
    {
        //id 0 is invalid
        audio_file_id++;
        if (audio_file_id == 0)
            audio_file_id = 1;

        esp_sound_start(audio_file_id, PROTO_FILE_WAV, file_size(audio_file));
        audio_file_opened = true;
    }
    else
    {
    	WARN("Cannot open sound %s file", filename);
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

    int32_t br = red_read(audio_file, buf + sizeof(proto_spi_header_t), requested_size);

    if (br == 0)
    {
        sound_close();
    }
    else
    {
        //only when error
        WARN("audio file not open for reading");
        br = 0;
    }

    if (br > 0)
    {
        //add header
        __align proto_spi_header_t hdr;
        hdr.packet_type = SPI_EP_SOUND;
        hdr.data_id = id;
        hdr.data_len = br;
        safe_memcpy(buf, &hdr,  + sizeof(proto_spi_header_t));

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
    red_close(audio_file);
    audio_file_opened = false;
}

void sound_stop()
{
    sound_close();
    esp_sound_stop();
}
