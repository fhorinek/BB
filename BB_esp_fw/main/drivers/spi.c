/*
 * spi.c
 *
 *  Created on: 4. 12. 2020
 *      Author: horinek
 */
#define DEBUG_LEVEL    DEBUG_DBG

#include "spi.h"


#include "driver/spi_slave.h"
#include "../protocol.h"
#include "../pipeline/sound.h"
#include "../linked_list.h"

static SemaphoreHandle_t spi_buffer_access;
static SemaphoreHandle_t spi_prepare_semaphore;

static uint16_t spi_data_to_send;

static uint16_t spi_tx_buffer_index;
DMA_ATTR static uint8_t spi_rx_buffer[SPI_BUFFER_SIZE];
DMA_ATTR static uint8_t spi_tx_buffer[SPI_BUFFER_SIZE];

void spi_prepare_buffer()
{
    xSemaphoreGive(spi_prepare_semaphore);
}

uint8_t * spi_acquire_buffer_ptr(uint32_t * size_avalible)
{
	xSemaphoreTake(spi_buffer_access, WAIT_INF);

    *size_avalible = SPI_BUFFER_SIZE - spi_tx_buffer_index;

    return &spi_tx_buffer[spi_tx_buffer_index];
}

void spi_release_buffer(uint32_t data_written)
{
	data_written = (data_written + 3) & ~3;
    spi_tx_buffer_index += data_written;
    xSemaphoreGive(spi_buffer_access);
}

void spi_setup_cb(spi_slave_transaction_t *trans)
{
//    DBG("spi_setup_cb: Buffer ready");

    protocol_send_spi_ready(spi_data_to_send);
}

void spi_trans_cb(spi_slave_transaction_t *trans)
{
//    DBG("spi_trans_cb: Transaction done");
}

uint16_t spi_send(uint8_t * data, uint16_t len)
{
    xSemaphoreTake(spi_buffer_access, WAIT_INF);

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
        memcpy((void *)&spi_tx_buffer[spi_tx_buffer_index], data, to_write);
        spi_tx_buffer_index += to_write;
    }

    xSemaphoreGive(spi_buffer_access);

    return to_write;
}
void spi_parse(uint8_t * data, uint16_t len)
{
	while (len > 0)
	{
		proto_spi_header_t * hdr = (proto_spi_header_t *)data;

		//advance buffer
		data += sizeof(proto_spi_header_t);
		len -= sizeof(proto_spi_header_t) + hdr->data_len;

		if (hdr->data_len == 0)
			return;

		//handle comunication
		if (hdr->packet_type == SPI_EP_SOUND)
		{
			pipe_sound_write(hdr->data_id, data, hdr->data_len);
		}

		if (hdr->packet_type == SPI_EP_FILE)
		{
        	ll_item_t * handle = ll_find_item(PROTO_FS_GET_FILE_RES, hdr->data_id);
        	if (handle != NULL)
        	{
        		xSemaphoreTake(handle->write, WAIT_INF);
        		handle->data_ptr = ps_malloc(hdr->data_len + 4);
        		*((uint32_t *)(handle->data_ptr)) = hdr->data_len;
        		memcpy(handle->data_ptr + 4, data, hdr->data_len);
        		xSemaphoreGive(handle->read);
        	}
		}


		//advance buffer
		uint16_t size = (hdr->data_len + 3) & ~3;
		data += size;
	}
}

void spi_task(void *pvParameters)
{
    while(1)
    {
        spi_slave_transaction_t t;
        memset(&t, 0, sizeof(t));

        //wait for the prepare request
        xSemaphoreTake(spi_prepare_semaphore, WAIT_INF);

        xSemaphoreTake(spi_buffer_access, WAIT_INF);

        t.length = SPI_BUFFER_SIZE * 8;
        t.tx_buffer = spi_tx_buffer;
        t.rx_buffer = spi_rx_buffer;

        spi_data_to_send = spi_tx_buffer_index;

        spi_slave_transmit(SPI_PORT, &t, WAIT_INF);

        //TODO: parse rx buffer
        //trans_len is in bits
        uint16_t len = t.trans_len / 8;

        DBG("SPI RX data: %u", len);
//        DUMP(t.rx_buffer, len);

        spi_parse(t.rx_buffer, len);

        spi_tx_buffer_index = 0;
        xSemaphoreGive(spi_buffer_access);
    }
}

void spi_init()
{
    spi_buffer_access = xSemaphoreCreateBinary();
    spi_prepare_semaphore = xSemaphoreCreateBinary();

    xSemaphoreGive(spi_buffer_access);

    //Configuration for the SPI bus
    spi_bus_config_t buscfg = {
            .mosi_io_num = SPI_MOSI,
            .miso_io_num = SPI_MISO,
            .sclk_io_num = SPI_SCLK,
            .quadwp_io_num = GPIO_NUM_NC,
            .quadhd_io_num = GPIO_NUM_NC,
			.max_transfer_sz = SPI_BUFFER_SIZE,
    };

    //Configuration for the SPI slave interface
    spi_slave_interface_config_t slvcfg = {
            .mode = 0,
            .spics_io_num = SPI_CS,
            .queue_size = 3,
            .flags = 0,
            .post_setup_cb = spi_setup_cb,
            .post_trans_cb = spi_trans_cb
    };

    //Initialize SPI slave interface
    uint8_t ret = spi_slave_initialize(SPI_PORT, &buscfg, &slvcfg, SPI_DMA_CHAN);
    assert(ret == ESP_OK);

    //create spi task
    xTaskCreate(spi_task, "spi_task", 512 * 4, NULL, 17, NULL);
}


