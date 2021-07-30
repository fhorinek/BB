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

#define API_POST_LEN	256
char * get_post_data(httpd_req_t * req)
{
    size_t recv_size = min(req->content_len, API_POST_LEN);
	char * data = ps_malloc(recv_size + 1);
    httpd_req_recv(req, data, recv_size);
	data[recv_size] = 0;

	return data;
}


esp_err_t api_handle_sound(httpd_req_t * req)
{
	char * data = get_post_data(req);

    char tone_key[16];
    char dura_key[16];

    int16_t cnt;

    tone_pair_t * pairs;

	if (read_post_int(data, "cnt", &cnt))
	{
		pairs = ps_malloc(sizeof(tone_pair_t *) * cnt);
		for (uint8_t i = 0; i < cnt; i++)
		{
			sprintf(tone_key, "tone_%u", i);
			sprintf(dura_key, "dura_%u", i);

			read_post_int(data, tone_key, &pairs[i].tone);
			read_post_int(data, dura_key, &pairs[i].dura);
		}

		vario_create_sequence(pairs, cnt);
		free(pairs);
	}
	free(data);


    httpd_resp_set_status(req, HTTPD_204);
    httpd_resp_send(req, NULL, 0);

	return ESP_OK;
}

#define API_FS_WAIT		500
static uint8_t api_fs_req = 0;

esp_err_t api_handle_list_fs(httpd_req_t * req)
{
	char * post_data = get_post_data(req);

    set_mime_type(req, ".json");
    output_buffer_t * ob = ob_create(req, OB_DEFAULT_SIZE);

	proto_fs_list_req_t data;
	if (read_post(post_data, "path", data.path, sizeof(data.path)))
	{
		int16_t tmp;
		if (!read_post_int(post_data, "filter", &tmp))
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

		ob_add(ob, "[", 1);

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
				snprintf(buff, sizeof(buff), "[\"%s\",%u]", res->name, res->type);

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

esp_err_t api_handle_get_file(httpd_req_t * req)
{
	char * post_data = get_post_data(req);

	proto_fs_get_file_req_t data;
	if (read_post(post_data, "path", data.path, sizeof(data.path)))
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

#define API_SAVE_FILE_CHUNK		1024
#define TAG "API"
esp_err_t api_handle_save_file(httpd_req_t * req)
{
    size_t recv_size = min(req->content_len, 10 * 1024);
	char * post_data = ps_malloc(recv_size + 1);
    httpd_req_recv(req, post_data, recv_size);
    post_data[recv_size] = 0;

    proto_fs_save_file_req_t data;
	if (read_post(post_data, "path", data.path, sizeof(data.path)))
	{
		char * file_buffer = ps_malloc(recv_size);
		read_post(post_data, "data", file_buffer, recv_size);
		uint32_t file_len = strlen(file_buffer);

		data.size = file_len;
		data.req_id = api_fs_req;
		api_fs_req++;

		protocol_send(PROTO_FS_SAVE_FILE_REQ, (void *)&data, sizeof(data));

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
			if (file_len - pos < chunk)
				chunk = file_len - pos;

			//add header
			proto_spi_header_t hdr;
			hdr.packet_type = SPI_EP_FILE;
			hdr.data_id = data.req_id;
			hdr.data_len = chunk;
			memcpy(buf, &hdr, sizeof(proto_spi_header_t));

			//add data
			memcpy(buf + sizeof(proto_spi_header_t), file_buffer + pos, chunk);

			spi_release_buffer(chunk + sizeof(proto_spi_header_t));
			spi_prepare_buffer();

			pos += chunk;

			if (pos >= file_len)
				break;
		}

		free(file_buffer);
	}

	char msg[] = "File saved";
	httpd_resp_send(req, msg, sizeof(msg) - 1);

	free(post_data);
	return ESP_OK;
}


