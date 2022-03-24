/*
 * flasher.c
 *
 *  Created on: 10. 2. 2021
 *      Author: horinek
 */

#include "flasher.h"

#include "esp_loader.h"
#include "stm32_port.h"

#include "gfx.h"
#include "lib/stm32-bootloader/bootloader.h"

uint32_t flasher_aligned(uint32_t size)
{
	return (size + 3) & ~3;
}

flasher_ret_t check_update_file(FIL * file)
{
    uint8_t buff[COPY_WORK_BUFFER_SIZE];

    file_header_t file_header;

    uint8_t stm_fw = false;
    uint8_t esp_parts = 0;
    uint16_t assets = 0;

    //rewind
    f_lseek(file, 0);

    UINT br;
    ASSERT(f_read(file, &file_header, sizeof(file_header_t), &br) == FR_OK);
    if (br != sizeof(file_header_t))
    {
        return flasher_unexpected_eof;
    }

    for (uint8_t i = 0; i < file_header.number_of_records; i++)
    {
        chunk_header_t chunk;

        ASSERT(f_read(file, &chunk, sizeof(chunk_header_t), &br) == FR_OK);
        if (br != sizeof(chunk_header_t))
        {
            return flasher_unexpected_eof;
        }

        if (chunk.addr == CHUNK_STM_ADDR)
        {
            if (Bootloader_CheckSize(chunk.size) != BL_OK)
                return flasher_wrong_size;
            else
            	stm_fw++;

        } else if (chunk.addr & CHUNK_FS_MASK)
        {
        	assets++;
        }
        else
        {
        	esp_parts++;
        }


        uint32_t crc;

        //reset crc unit
        __HAL_CRC_DR_RESET(&hcrc);
        uint32_t pos = 0;

        uint32_t aligned_size = flasher_aligned(chunk.size);

        while (aligned_size > pos)
        {
            uint32_t to_read = aligned_size - pos;
            if (to_read > COPY_WORK_BUFFER_SIZE)
                to_read = COPY_WORK_BUFFER_SIZE;

            ASSERT(f_read(file, buff, to_read, &br) == FR_OK);

            crc = HAL_CRC_Accumulate(&hcrc, (uint32_t *)buff, br);

            gfx_draw_progress(f_tell(file) / (float)f_size(file));

            if (br == 0)
            {
                ERR(" chunk %lu %s unexpected eof at %lu", chunk.addr, chunk.name, pos);
                return flasher_unexpected_eof;
            }

            pos += br;
        }

        if (chunk.addr != CHUNK_STM_ADDR)
        {
			crc = HAL_CRC_Accumulate(&hcrc, chunk.name, sizeof(chunk.name));
			crc = HAL_CRC_Accumulate(&hcrc, &chunk.addr, sizeof(chunk.addr));
			crc = HAL_CRC_Accumulate(&hcrc, &chunk.size, sizeof(chunk.size));
        }

        crc ^= 0xFFFFFFFF;

        if (crc != chunk.crc)
        {
            ERR(" chunk %08X %s crc fail", chunk.addr, chunk.name);
            return flasher_crc_failed;
            break;
        }
    }

    INFO("STM: %u, ESP: %u, ASSETS: %u", stm_fw, esp_parts, assets);

    if (stm_fw == 1 && esp_parts > 2)
    	return flasher_ok;
    else
    	return flasher_not_valid;
}

#define ESP_DMA_BUFFER_SIZE 256
uint8_t esp_rx_buffer[ESP_DMA_BUFFER_SIZE];

static uint16_t esp_read_index = 0;

uint16_t esp_get_waiting()
{
    uint16_t write_index = ESP_DMA_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(esp_uart->hdmarx);

    //Get number of bytes waiting in buffer
    if (esp_read_index > write_index)
    {
        return ESP_DMA_BUFFER_SIZE - esp_read_index + write_index;
    }
    else
    {
        return write_index - esp_read_index;
    }
}

uint8_t esp_read_byte()
{
    uint8_t byte = esp_rx_buffer[esp_read_index];
    esp_read_index = (esp_read_index + 1) % ESP_DMA_BUFFER_SIZE;
    return byte;
}

uint16_t esp_read_bytes(uint8_t * data, uint16_t len, uint32_t timeout)
{
    uint32_t end = HAL_GetTick() + timeout;
    uint16_t readed = 0;

    while (len > readed)
    {
        if (esp_get_waiting() > 0)
        {
            data[readed] = esp_read_byte();
            readed++;
        }

        if (HAL_GetTick() > end)
            return readed;
    }

    return readed;
}

flasher_ret_t esp_flash_write_file(FIL * file)
{
    uint8_t work_buff[COPY_WORK_BUFFER_SIZE];

    GpioSetDirection(ESP_BOOT, OUTPUT, GPIO_NOPULL);
    GpioSetDirection(ESP_EN, OUTPUT, GPIO_NOPULL);

    HAL_UART_Receive_DMA(esp_uart, esp_rx_buffer, ESP_DMA_BUFFER_SIZE);

    loader_port_change_baudrate(115200);

    gfx_draw_status(GFX_STATUS_UPDATE, "ESP programming");
    esp_loader_connect_args_t connect_config = ESP_LOADER_CONNECT_DEFAULT();
    esp_loader_error_t err = esp_loader_connect(&connect_config);
    if (err != ESP_LOADER_SUCCESS)
    {
        return flasher_unable_to_connect;
    }

    DBG("Changing baudrate");
    err = esp_loader_change_baudrate(HIGHER_BAUDRATE);
    if (err != ESP_LOADER_SUCCESS)
    {
        return flasher_unable_to_set_baudrate;
    }
    loader_port_change_baudrate(HIGHER_BAUDRATE);

    file_header_t file_header;

    //rewind
    f_lseek(file, 0);

    UINT br;
    ASSERT(f_read(file, &file_header, sizeof(file_header_t), &br) == FR_OK);
    if (br != sizeof(file_header_t))
    {
        return flasher_unexpected_eof;
    }

    for (uint8_t i = 0; i < file_header.number_of_records; i++)
    {
        chunk_header_t chunk;

        ASSERT(f_read(file, &chunk, sizeof(chunk_header_t), &br) == FR_OK);
        if (br != sizeof(chunk_header_t))
        {
            return flasher_unexpected_eof;
        }

        //skip stm fw and assets
        if (chunk.addr == CHUNK_STM_ADDR || chunk.addr & CHUNK_FS_MASK)
        {
            f_lseek(file, f_tell(file) + flasher_aligned(chunk.size));
            continue;
        }

        uint32_t pos = 0;

        esp_loader_error_t err;

        ASSERT(ESP_PACKET_SIZE < COPY_WORK_BUFFER_SIZE);

        DBG("Writing 0x%08X %8u %s", chunk.addr, chunk.size, chunk.name);

        char text[64];
        sprintf(text, "ESP %s", chunk.name);
        gfx_draw_status(GFX_STATUS_UPDATE, text);

        err = esp_loader_flash_start(chunk.addr, chunk.size, ESP_PACKET_SIZE);
        if (err != ESP_LOADER_SUCCESS)
        {
            ERR("Programming error %u", err);
            return flasher_unable_to_program;
        }

        while (chunk.size > pos)
        {
            uint32_t to_read = chunk.size - pos;
            if (to_read > ESP_PACKET_SIZE)
                to_read = ESP_PACKET_SIZE;

            ASSERT(f_read(file, work_buff, to_read, &br) == FR_OK);

            gfx_draw_progress(f_tell(file) / (float)f_size(file));

            if (br == 0)
            {
                return flasher_unexpected_eof;
            }

            err = esp_loader_flash_write(work_buff, to_read);
            if (err != ESP_LOADER_SUCCESS)
            {
                ERR("Packet could not be written! Error %d.", err);
                return flasher_unable_to_program;
            }

            pos += br;
        }

        err = esp_loader_flash_verify();

        if (err != ESP_LOADER_SUCCESS)
        {
            return flasher_crc_failed;
        }
    }

    DBG("Done");

    return flasher_ok;
}


