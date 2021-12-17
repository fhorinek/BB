/*
 * spi.c
 *
 *  Created on: 4. 12. 2020
 *      Author: horinek
 */
#define DEBUG_LEVEL    DEBUG_DBG

#include "esp.h"
#include "drivers/esp/protocol.h"
#include "drivers/esp/file.h"

osSemaphoreId_t spi_buffer_access;
osSemaphoreId_t spi_start_semaphore;
osSemaphoreId_t spi_dma_done;

static uint16_t spi_data_to_read;

static uint16_t spi_tx_buffer_index;
static __align uint8_t spi_rx_buffer[SPI_BUFFER_SIZE];
static __align uint8_t spi_tx_buffer[SPI_BUFFER_SIZE];

void spi_start_transfer(uint16_t size_to_read)
{
    spi_data_to_read = size_to_read;
    osSemaphoreRelease(spi_start_semaphore);
}

uint16_t esp_spi_send(uint8_t * data, uint16_t len)
{
    osSemaphoreAcquire(spi_buffer_access, WAIT_INF);

    uint16_t free_space = SPI_BUFFER_SIZE - spi_tx_buffer_index;
    uint16_t to_write;

    if (free_space > len)
    {
        to_write = len;
    }
    else
    {
        to_write = free_space;
    }

    if (to_write > 0)
    {
        safe_memcpy((void *)&spi_tx_buffer[spi_tx_buffer_index], data, to_write);
        spi_tx_buffer_index += to_write;
    }

    osSemaphoreRelease(spi_buffer_access);

    return to_write;
}

uint8_t * esp_spi_acquire_buffer_ptr(uint16_t * size_avalible)
{
    osSemaphoreAcquire(spi_buffer_access, WAIT_INF);

    *size_avalible = SPI_BUFFER_SIZE - spi_tx_buffer_index;

    return &spi_tx_buffer[spi_tx_buffer_index];
}

void esp_spi_release_buffer(uint16_t data_written)
{
	data_written = (data_written + 3) & ~3;
    spi_tx_buffer_index += data_written;
    osSemaphoreRelease(spi_buffer_access);
}

void spi_dma_done_cb()
{
    osSemaphoreRelease(spi_dma_done);
}

void esp_parse_spi(uint8_t * data, uint16_t len)
{
    while (len > 0)
    {
        proto_spi_header_t * hdr = (proto_spi_header_t *)data;

        //advance buffer
        data += sizeof(proto_spi_header_t);
        len -= sizeof(proto_spi_header_t) + hdr->data_len;

        if (hdr->data_len == 0)
            return;

        switch (hdr->packet_type)
        {
            case(SPI_EP_DOWNLOAD):
                download_slot_process_data(hdr->data_id, data, hdr->data_len);
            break;

            case(SPI_EP_FILE):
				file_get_file_data(hdr->data_id, data, hdr->data_len);
			break;
        }

        //advance buffer
        uint16_t size = (hdr->data_len + 3) & ~3;
        data += size;
    }
}

void thread_esp_spi_start(void * argument)
{
    spi_buffer_access = osSemaphoreNew(1, 0, NULL);
    spi_start_semaphore = osSemaphoreNew(1, 0, NULL);
    spi_dma_done = osSemaphoreNew(1, 0, NULL);

    vQueueAddToRegistry(spi_buffer_access, "spi_buffer_access");
    vQueueAddToRegistry(spi_start_semaphore, "spi_start_semaphore");
    vQueueAddToRegistry(spi_dma_done, "spi_dma_done");

    osSemaphoreRelease(spi_buffer_access);

    while (1)
    {
        osSemaphoreAcquire(spi_start_semaphore, WAIT_INF);

        osSemaphoreAcquire(spi_buffer_access, WAIT_INF);

        uint16_t spi_data_to_send = spi_tx_buffer_index;
        if (spi_data_to_read > spi_data_to_send)
            spi_data_to_send = spi_data_to_read;

        ASSERT(spi_data_to_send > 0);

        uint8_t res = HAL_SPI_TransmitReceive_DMA(esp_spi, spi_tx_buffer, spi_rx_buffer, spi_data_to_send);
        if (res != HAL_OK)
        {
            DBG("res = %02X", res);
        }

        osSemaphoreAcquire(spi_dma_done, WAIT_INF);

        DBG("SPI RX data: %u", spi_data_to_read);
        //DUMP(spi_rx_buffer, spi_data_to_read);

        //parse spi data here (new thread??)
        esp_parse_spi(spi_rx_buffer, spi_data_to_read);

        spi_tx_buffer_index = 0;
        osSemaphoreRelease(spi_buffer_access);
    }

    INFO("Done");
    osThreadSuspend(thread_esp_spi);
}

