/*
 * server.c
 *
 *  Created on: 5. 5. 2021
 *      Author: horinek
 */

#include "server.h"

#include "esp_spiffs.h"

#include "file.h"
#include "render.h"
#include "api.h"

static httpd_handle_t server = NULL;

static const char * TAG = "Server";

esp_err_t index_get_handler(httpd_req_t *req)
{
	render_page(req, "/index.htm");

    return ESP_OK;
}

esp_err_t page_get_handler(httpd_req_t *req)
{
	render_page(req, (char *)req->uri);

    return ESP_OK;
}

esp_err_t file_get_handler(httpd_req_t *req)
{
	send_file(req, (char *)req->uri);

    return ESP_OK;
}

static const httpd_uri_t index_handler = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = index_get_handler,
    .user_ctx  = NULL,
};

static const httpd_uri_t page_handler = {
    .uri       = "*.htm",
    .method    = HTTP_GET,
    .handler   = page_get_handler,
    .user_ctx  = NULL,
};

static const httpd_uri_t api_handler = {
    .uri       = "/api/sound",
    .method    = HTTP_POST,
    .handler   = api_sound,
    .user_ctx  = NULL,
};

static const httpd_uri_t api_fs_list = {
    .uri       = "/api/list_fs",
    .method    = HTTP_POST,
    .handler   = api_list_fs,
    .user_ctx  = NULL,
};

static const httpd_uri_t api_fs_get_file = {
    .uri       = "/api/get_file",
    .method    = HTTP_POST,
    .handler   = api_get_file,
    .user_ctx  = NULL,
};

static const httpd_uri_t api_fs_save_file = {
    .uri       = "/api/save_file",
    .method    = HTTP_POST,
    .handler   = api_save_file,
    .user_ctx  = NULL,
};

static const httpd_uri_t api_fake_gnss_h = {
    .uri       = "/api/fake_gnss",
    .method    = HTTP_POST,
    .handler   = api_fake_gnss,
    .user_ctx  = NULL,
};


static const httpd_uri_t file_handler = {
    .uri       = "*",
    .method    = HTTP_GET,
    .handler   = file_get_handler,
    .user_ctx  = NULL,
};

void server_init()
{
    esp_vfs_spiffs_conf_t conf = {
		.base_path = "",
		.partition_label = NULL,
		.max_files = 5,
		.format_if_mount_failed = false
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK)
    {
		if (ret == ESP_FAIL) {
			ESP_LOGE(TAG, "Failed to mount or format filesystem");
		} else if (ret == ESP_ERR_NOT_FOUND) {
			ESP_LOGE(TAG, "Failed to find SPIFFS partition");
		} else {
			ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
		}

		system_status.server_ok = false;
		return;
    }
    else
    {
    	system_status.server_ok = true;
    }

	httpd_config_t config = HTTPD_DEFAULT_CONFIG();
	config.uri_match_fn = httpd_uri_match_wildcard;

    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_register_uri_handler(server, &index_handler);
        httpd_register_uri_handler(server, &api_handler);
        httpd_register_uri_handler(server, &page_handler);
        httpd_register_uri_handler(server, &api_fs_list);
        httpd_register_uri_handler(server, &api_fs_get_file);
        httpd_register_uri_handler(server, &api_fs_save_file);
        httpd_register_uri_handler(server, &api_fake_gnss_h);
        httpd_register_uri_handler(server, &file_handler);

//        httpd_register_basic_auth(server);
    }
}


