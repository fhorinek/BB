/*
 * download.c
 *
 *  Created on: 19. 4. 2021
 *      Author: horinek
 */

#define DEBUG_LEVEL	DBG_DEBUG
#include "download.h"

#include "protocol.h"
#include "drivers/spi.h"
#include "wifi.h"

#include "esp_http_client.h"

static const char * TAG = "Download";

static uint32_t stop_flag = 0;

void download_start(uint8_t data_id)
{
	ASSERT(data_id < 32);
	stop_flag &= ~(1 << data_id);
}

void download_stop(uint8_t data_id)
{
	ASSERT(data_id < 32);
	stop_flag |= 1 << data_id;
}

bool download_is_canceled(uint8_t data_id)
{
	ASSERT(data_id < 32);
	return (stop_flag & 1 << data_id) > 0;
}

void download_process_url(proto_download_url_t * packet)
{
	proto_download_info_t data;
	esp_http_client_config_t config = {0};
	config.url = packet->url;

	esp_http_client_handle_t client = esp_http_client_init(&config);

	esp_err_t ret = esp_http_client_open(client, 0);
	INFO("esp_http_client_open = %d", ret);
	if (ret == ESP_OK)
	{
		ESP_ERROR_CHECK(esp_http_client_write(client, NULL, 0));

		int len = esp_http_client_fetch_headers(client);
		INFO("esp_http_client_fetch_headers = %d", len);
		uint16_t status = esp_http_client_get_status_code(client);

		INFO("esp_http_client_get_status_code = %u", status);
		if (status == 200) //OK
		{
			if (esp_http_client_is_chunked_response(client))
			{
				esp_http_client_get_chunk_length(client, &len);
				INFO("esp_http_client_get_chunk_length = %d", len);
			}

			INFO("size = %d", len);

			uint32_t size = len;

			data.end_point = packet->data_id;
			data.result = PROTO_DOWNLOAD_OK;
			data.size = size;
			protocol_send(PROTO_DOWNLOAD_INFO, (void *)&data, sizeof(data));

			uint32_t pos = 0;
			while(pos < size || size == 0)
			{
				uint32_t free_space;
				uint8_t * buf = spi_acquire_buffer_ptr(&free_space);
				if (free_space <= sizeof(proto_spi_header_t))
				{
					spi_release_buffer(0);
					ESP_LOGI(TAG, "Nothing to do!");
					taskYIELD();
					continue;
				}

				uint32_t requested_size;

				INFO("buffer_free_space %u", free_space);

				if (size != 0)
				{
					requested_size = size - pos;
					if (free_space < requested_size + sizeof(proto_spi_header_t))
						requested_size = free_space - sizeof(proto_spi_header_t);
				}
				else
				{
					requested_size = free_space - sizeof(proto_spi_header_t);
				}

				INFO("%u/%u + %u", pos, size, requested_size);

				int16_t readed = esp_http_client_read(client, (char *)(buf + sizeof(proto_spi_header_t)), requested_size);

				//add header
				proto_spi_header_t hdr;
				hdr.packet_type = SPI_EP_DOWNLOAD;
				hdr.data_id = packet->data_id;
				hdr.data_len = readed;
				memcpy(buf, &hdr, sizeof(proto_spi_header_t));

				if (requested_size != readed)
				{
					if (size != 0)
						ERR("requested_size != readed %u != %d", requested_size, readed);
					else
						INFO("file end at %u bytes", pos + readed);

					download_stop(packet->data_id);
				}

				spi_release_buffer(readed + sizeof(proto_spi_header_t));
				spi_prepare_buffer();

				pos += readed;

				if (download_is_canceled(packet->data_id))
					break;
			}

			task_sleep(10);

			//to sync spi thread
			uint32_t free_space;
			spi_acquire_buffer_ptr(&free_space);
			spi_release_buffer(0);

			data.end_point = packet->data_id;
			data.result = PROTO_DOWNLOAD_DONE;
			data.size = pos;
			protocol_send(PROTO_DOWNLOAD_INFO, (void *)&data, sizeof(data));

		}
		else
		{

			data.end_point = packet->data_id;
			data.result = PROTO_DOWNLOAD_NOT_FOUND;
			data.size = 0;
			protocol_send(PROTO_DOWNLOAD_INFO, (void *)&data, sizeof(data));
		}

	}
	else
	{
		data.end_point = packet->data_id;
		data.result = PROTO_DOWNLOAD_NO_CONNECTION;
		data.size = 0;
		protocol_send(PROTO_DOWNLOAD_INFO, (void *)&data, sizeof(data));
	}

	esp_http_client_close(client);
	esp_http_client_cleanup(client);

	INFO("download_url %s done", packet->url);
}

void download_url(proto_download_url_t * packet)
{
	INFO("download_url start");

	download_start(packet->data_id);

	if (wifi_is_connected()) {
		download_process_url(packet);
	} else {
		proto_download_info_t data;
		data.end_point = packet->data_id;
		data.result = PROTO_DOWNLOAD_NO_CONNECTION;
		data.size = 0;
		protocol_send(PROTO_DOWNLOAD_INFO, (void *)&data, sizeof(data));

		WARN("download_url %s failed, no WIFI connection", packet->url);
	}

	download_stop(packet->data_id);

	free(packet);

	vTaskDelete(NULL);
}


