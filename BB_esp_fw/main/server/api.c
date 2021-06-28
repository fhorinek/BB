/*
 * api.c
 *
 *  Created on: 7. 5. 2021
 *      Author: horinek
 */

#include "api.h"

#include "../pipeline/vario.h"

#define API_SOUND_POST_LEN	256
esp_err_t api_handle_sound(httpd_req_t * req)
{

    size_t recv_size = min(req->content_len, API_SOUND_POST_LEN);
	char * data = ps_malloc(recv_size);
    int ret = httpd_req_recv(req, data, recv_size);

    char tone_key[16];
    char dura_key[16];

    int16_t tone, dura;
    int16_t cnt;

	if (read_post_int(data, "cnt", &cnt))
	{
		tone_part_t ** new_tones = ps_malloc(sizeof(tone_part_t *) * cnt);

		for (uint8_t i = 0; i < cnt; i++)
		{
			sprintf(tone_key, "tone_%u", i);
			sprintf(dura_key, "dura_%u", i);

			read_post_int(data, tone_key, &tone);
			read_post_int(data, dura_key, &dura);

			INFO("%u", i);
			INFO("%s = %u", tone_key, tone);
			INFO("%s = %u", dura_key, dura);

			if (dura > 0)
				new_tones[i] = vario_create_part(tone, dura);
		}

		pipe_vario_replace(new_tones, cnt);
	}

    free(data);

    httpd_resp_set_status(req, HTTPD_204);
    httpd_resp_send(req, NULL, 0);

	return ESP_OK;
}

#define API_FS_POST_LEN	140

static uint8_t api_fs_req = 0;

esp_err_t api_list_files(httpd_req_t * req)
{
    size_t recv_size = min(req->content_len, API_FS_POST_LEN);
	char * post_data = ps_malloc(recv_size);
    int ret = httpd_req_recv(req, post_data, recv_size);

    output_buffer_t * ob = ob_create(req, OB_DEFAULT_SIZE);

	proto_fs_list_req data;
	if (read_post(post_data, "path", data.path, sizeof(data.path)))
	{
		if (!read_post_int(post_data, "filter", &data.filter))
			data.filter = PROTO_FS_TYPE_FILE | PROTO_FS_TYPE_FOLDER;
		data.req_id = api_fs_req;

		protocol_send(PROTO_FS_LIST_REQ, (void *)&data, sizeof(data));




	}

	return ESP_OK;
}

esp_err_t api_get_file(httpd_req_t * req)
{

}

esp_err_t api_save_file(httpd_req_t * req)
{

}


