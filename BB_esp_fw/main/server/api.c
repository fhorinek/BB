/*
 * api.c
 *
 *  Created on: 7. 5. 2021
 *      Author: horinek
 */
#define DEBUG_LEVEL	DBG_DEBUG

#include "api.h"

#include "file.h"

#include "../pipeline/vario.h"
#include "../server/ob.h"
#include "../protocol.h"
#include "../linked_list.h"
#include "../drivers/spi.h"

#include "mbedtls/base64.h"

#define API_POST_LEN	256
char * get_post_data(httpd_req_t * req)
{
    size_t recv_size = min(req->content_len, API_POST_LEN);
	char * data = ps_malloc(recv_size + 1);
    httpd_req_recv(req, data, recv_size);
	data[recv_size] = 0;

	return data;
}


esp_err_t api_sound(httpd_req_t * req)
{
	char * data = get_post_data(req);

    char tone_key[16];
    char dura_key[16];

    int16_t cnt;

    tone_pair_t * pairs = NULL;

	if (read_post_int16(data, "cnt", &cnt))
	{
		if (cnt > 0)
		{
			pairs = ps_malloc(sizeof(tone_pair_t *) * cnt);
			for (uint8_t i = 0; i < cnt; i++)
			{
				sprintf(tone_key, "tone_%u", i);
				sprintf(dura_key, "dura_%u", i);

				read_post_int16(data, tone_key, &pairs[i].tone);
				read_post_int16(data, dura_key, &pairs[i].dura);
			}
		}

		vario_create_sequence(pairs, cnt);

		if (pairs != NULL)
			free(pairs);
	}
	free(data);


    httpd_resp_set_status(req, HTTPD_204);
    httpd_resp_send(req, NULL, 0);

	return ESP_OK;
}

#define API_FS_WAIT		500
static uint8_t api_fs_req = 0;

esp_err_t api_list_fs(httpd_req_t * req)
{
	char * post_data = get_post_data(req);

    set_mime_type(req, ".json");
    output_buffer_t * ob = ob_create(req, OB_DEFAULT_SIZE);

    ob_add(ob, "[", 1);

	proto_fs_list_req_t data;
	if (read_post(post_data, "path", data.path, sizeof(data.path)) >= 0)
	{
		int16_t tmp;
		if (!read_post_int16(post_data, "filter", &tmp))
		{
			data.filter = PROTO_FS_TYPE_FILE | PROTO_FS_TYPE_FOLDER;
		}
		else
		{
			data.filter = tmp;
		}

		data.req_id = api_fs_req;
		api_fs_req++;

		ll_item_t * handle = ll_add_item(PROTO_FS_LIST_RES, data.req_id, sizeof(proto_fs_list_res_t));
		protocol_send(PROTO_FS_LIST_REQ, (void *)&data, sizeof(data));

		//wait for response
		DBG("listing '%s'", data.path);

		bool first = true;
		while (true)
		{

			if (!xSemaphoreTake(handle->read, API_FS_WAIT))
			{
				//error
				WARN("FS operation timeout!");
				break;
			}

			proto_fs_list_res_t * res = (proto_fs_list_res_t *)handle->data_ptr;

			if (!(res->type & PROTO_FS_TYPE_END))
			{
				//process
				INFO(">>%s", res->name);
				char buff[45];
				snprintf(buff, sizeof(buff), "[\"%s\",%u,%u]", res->name, res->size, res->type);

				if (!first)
					ob_add(ob, ",", 1);
				else
					first = false;

				ob_add(ob, buff, strlen(buff));
			}
			else
			{
				INFO("END");
				break;
			}

			xSemaphoreGive(handle->write);
		}

		ll_delete_item(handle);
	}
	free(post_data);

	ob_add(ob, "]", 1);
	//close connection
	ob_flush(ob);

	return ESP_OK;
}

esp_err_t api_get_file(httpd_req_t * req)
{
	char * post_data = get_post_data(req);

	httpd_resp_set_type(req, "application/octet-stream");

	proto_fs_get_file_req_t data;
	if (read_post(post_data, "path", data.path, sizeof(data.path)) >= 0)
	{
		set_mime_type(req, data.path);
		output_buffer_t * ob = ob_create(req, OB_DEFAULT_SIZE);

		data.req_id = api_fs_req;
		api_fs_req++;
		data.chunk_size = 1024;

		ll_item_t * handle = ll_add_item(PROTO_FS_GET_FILE_RES, data.req_id, 0);
		protocol_send(PROTO_FS_GET_FILE_REQ, (void *)&data, sizeof(data));

		while (true)
		{
			if (!xSemaphoreTake(handle->read, API_FS_WAIT))
			{
				//error
				WARN("FS operation timeout!");
				break;
			}

			uint32_t rx_size = 0;

			if (handle->data_ptr != NULL)
			{
				rx_size = *((uint32_t *)handle->data_ptr);
				ob_add(ob, handle->data_ptr + 4, rx_size);

				free(handle->data_ptr);
				handle->data_ptr = NULL;
			}
			else
			{
				//file not found!
			}

			xSemaphoreGive(handle->write);

			//last packet
			if (rx_size != data.chunk_size)
				break;
		}
		ll_delete_item(handle);

		//close connection
		ob_flush(ob);
	}
	free(post_data);


	return ESP_OK;
}

#define API_SAVE_FILE_CHUNK		(3 * 1024)
#define TAG "API"

esp_err_t api_save_file(httpd_req_t * req)
{
	ESP_LOGI(TAG, "api_save_file");

    size_t recv_size = min(req->content_len, 20 * 1024);
    ESP_LOGI(TAG, " recv_size %d", recv_size);

	char * post_data = ps_malloc(recv_size + 1);
    httpd_req_recv(req, post_data, recv_size);
    post_data[recv_size] = 0;

	char * file_buffer = ps_malloc(recv_size + 1);
	int32_t data_size;

	if ((data_size = read_post(post_data, "data", file_buffer, recv_size)) > 0)
	{
		ESP_LOGI(TAG, " data_size plain %d", data_size);
	}
	else if (((data_size = read_post(post_data, "data64", file_buffer, recv_size)) > 0))
	{
		//buffer swap! we do not need post_data anymore, so we reuse it
		char * tmp = ps_malloc(data_size);

		size_t outlen;
		int ret = mbedtls_base64_decode((uint8_t *)tmp, data_size, &outlen, (uint8_t *)file_buffer, data_size);

		if (ret == 0)
		{
			free(file_buffer);
			file_buffer = tmp;
			file_buffer[outlen] = 0;
			data_size = outlen;
		}
		else
		{
			data_size = 0;
		}

		ESP_LOGI(TAG, " data_size base64 %d", data_size);
	}

	if (data_size <= 0)
	{
		ERR("No data send");
		//no data send
		free(file_buffer);
		free(post_data);

		return ESP_FAIL;
	}


	int32_t file_size;
	if (!read_post_int32(post_data, "size", &file_size))
	{
		//no extra size parameter, we assume that data_size is file_size
		file_size = data_size;
	}

	int32_t file_index;
	if (!read_post_int32(post_data, "index", &file_index))
	{
		//no index, start from 0
		file_index = 0;
	}

    proto_fs_save_file_req_t data;

	int32_t tmp;
	if (read_post_int32(post_data, "file_id", &tmp))
	{
		data.req_id = tmp;
	}
	else if (read_post(post_data, "path", data.path, sizeof(data.path)) > 0)
	{
		//no file_is, create file
		data.size = file_size;
		data.req_id = api_fs_req;
		api_fs_req++;

		protocol_send(PROTO_FS_SAVE_FILE_REQ, (void *)&data, sizeof(data));
	}
	else
	{
		free(file_buffer);
		free(post_data);

		return ESP_FAIL;
	}

	uint16_t pos = 0;
	while(1)
	{
		uint32_t free_space;
		uint8_t * buf = spi_acquire_buffer_ptr(&free_space);
		if (free_space <= sizeof(proto_spi_header_t) + API_SAVE_FILE_CHUNK)
		{
			spi_release_buffer(0);
			ESP_LOGI(TAG, "Nothing to do!");
			taskYIELD();
			continue;
		}
		uint16_t chunk = API_SAVE_FILE_CHUNK;
		if (data_size - pos < chunk)
			chunk = data_size - pos;

		//add header
		proto_spi_header_t hdr;
		hdr.packet_type = SPI_EP_FILE;
		hdr.data_id = data.req_id;
		hdr.data_len = chunk;
		memcpy(buf, &hdr, sizeof(proto_spi_header_t));

		//add data
		memcpy(buf + sizeof(proto_spi_header_t), file_buffer + pos, chunk);

		spi_release_buffer(chunk + sizeof(proto_spi_header_t));
		spi_prepare_buffer(0);

		pos += chunk;

		if (pos >= data_size)
			break;
	}

	free(file_buffer);

	file_index += data_size;

	if (file_index == file_size)
	{
		char msg[] = "{\"done\":true}";
		httpd_resp_send(req, msg, sizeof(msg) - 1);
	}
	else
	{
		char msg[64];
		uint8_t len = snprintf(msg, sizeof(msg), "{\"done\":false,\"file_id\":%u,\"index\":%u, \"rx_len\":%u}", data.req_id, file_index, data_size);
		httpd_resp_send(req, msg, len);
	}

	free(post_data);
	return ESP_OK;
}

esp_err_t api_del_file(httpd_req_t * req)
{
	char * post_data = get_post_data(req);

	proto_fs_delete_file_req_t data;
	if (read_post(post_data, "path", data.path, sizeof(data.path)) > 0)
	{
		ESP_LOGI(TAG, "Delete file '%s'", data.path);
		protocol_send(PROTO_FS_DELETE_FILE_REQ, (uint8_t *)&data, sizeof(data));
	}
	free(post_data);

    httpd_resp_set_status(req, HTTPD_204);
    httpd_resp_send(req, NULL, 0);

	return ESP_OK;
}


esp_err_t api_fake_gnss(httpd_req_t * req)
{
	char * post_data = get_post_data(req);

	proto_fake_gnss_t data;

	read_post_int32(post_data, "time", &data.timestamp);
	read_post_int32(post_data, "lat", &data.lat);
	read_post_int32(post_data, "lon", &data.lon);
	read_post_int16(post_data, "speed", &data.speed);
	read_post_int16(post_data, "heading", &data.heading);

	int16_t tmp;

	read_post_int16(post_data, "fix", &tmp);
	data.fix = tmp;

	protocol_send(PROTO_FAKE_GNSS, (void *)&data, sizeof(data));

	free(post_data);

    httpd_resp_set_status(req, HTTPD_204);
    httpd_resp_send(req, NULL, 0);

	return ESP_OK;
}


