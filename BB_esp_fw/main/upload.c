/*
 * upload.c
 *
 *  Created on: 31.03.2022
 *      Author: simonseyer
 */

#include "upload.h"

#include "protocol.h"
#include "drivers/spi.h"
#include "wifi.h"
#include "linked_list.h"

#include "esp_http_client.h"

#define UPLOAD_FS_WAIT     500

static uint32_t stop_flag = 0;

void upload_start(uint8_t data_id)
{
    ASSERT(data_id < 32);
    stop_flag &= ~(1 << data_id);
}

void upload_stop(uint8_t data_id)
{
    ASSERT(data_id < 32);
    stop_flag |= 1 << data_id;
}

bool upload_is_canceled(uint8_t data_id)
{
    ASSERT(data_id < 32);
    return (stop_flag & 1 << data_id) > 0;
}

void upload_process_request(proto_upload_request_t * packet)
{
    proto_upload_info_t info;
    info.end_point = packet->data_id;

    esp_http_client_config_t config = {0};
    config.url = packet->url;
    config.method = HTTP_METHOD_POST;


    uint32_t total_size = 0;

    esp_http_client_handle_t client = esp_http_client_init(&config);

    // TODO: detect content type automatically based on file extension (or pass via packet?)
    esp_http_client_set_header(client, "Content-Type", "text/plain");

    esp_err_t ret = esp_http_client_open(client, packet->file_size);
    INFO("esp_http_client_open = %s", esp_err_to_name(ret));

    if (ret == ESP_OK)
    {
        proto_fs_get_file_req_t file_request;
        strncpy(file_request.path, packet->file_path, PROTO_FS_PATH_LEN);
        file_request.req_id = packet->data_id;
        file_request.chunk_size = 1024;

        ll_item_t * handle = ll_add_item(PROTO_FS_GET_FILE_RES, file_request.req_id, 0);
        protocol_send(PROTO_FS_GET_FILE_REQ, (void *)&file_request, sizeof(file_request));

        while (true)
        {
            if (!xSemaphoreTake(handle->read, UPLOAD_FS_WAIT))
            {
                //error
                WARN("FS operation timeout!");
                break;
            }

            uint32_t rx_size = 0;

            if (handle->data_ptr != NULL)
            {
                rx_size = *((uint32_t *)handle->data_ptr);
                int wlen = esp_http_client_write(client, handle->data_ptr + 4, rx_size);
                if (wlen != rx_size)
                {
                    WARN("Didn't write all data: %u of %u", wlen, rx_size);
                }

                total_size += rx_size;

                free(handle->data_ptr);
                handle->data_ptr = NULL;
            }
            else
            {
                //file not found!
            }

            xSemaphoreGive(handle->write);

            if (rx_size == file_request.chunk_size)
            {
                // Send upload info
                info.result = PROTO_UPLOAD_OK;
                info.size = total_size;
                protocol_send(PROTO_UPLOAD_INFO, (void *)&info, sizeof(info));
            }
            else
            {
                //last packet
                break;
            }

            if (upload_is_canceled(packet->data_id))
                break;
        }
        ll_delete_item(handle);

        int len = esp_http_client_fetch_headers(client);
        INFO("esp_http_client_fetch_headers = %d", len);
        uint16_t status = esp_http_client_get_status_code(client);

        INFO("esp_http_client_get_status_code = %u", status);
        if (status == 200) //OK
        {
            info.result = PROTO_UPLOAD_DONE;
            info.size = total_size;
            protocol_send(PROTO_UPLOAD_INFO, (void *)&info, sizeof(info));
        }
        else
        {
            info.result = PROTO_UPLOAD_FAILED;
            info.size = total_size;
            protocol_send(PROTO_UPLOAD_INFO, (void *)&info, sizeof(info));
        }
    }
    else
    {
        info.result = PROTO_UPLOAD_NO_CONNECTION;
        info.size = total_size;
        protocol_send(PROTO_UPLOAD_INFO, (void *)&info, sizeof(info));
    }

    esp_http_client_close(client);
    esp_http_client_cleanup(client);

    INFO("upload_url %s done", packet->url);

}

void upload_file(proto_upload_request_t * packet)
{
    INFO("upload_url start");

    upload_start(packet->data_id);

    if (wifi_is_connected()) {
        upload_process_request(packet);
    } else {
        proto_upload_info_t data;
        data.end_point = packet->data_id;
        data.result = PROTO_UPLOAD_NO_CONNECTION;
        data.size = 0;
        protocol_send(PROTO_UPLOAD_INFO, (void *)&data, sizeof(data));

        WARN("upload_url %s failed, no WIFI connection", packet->url);
    }

    upload_stop(packet->data_id);

    free(packet);

    vTaskDelete(NULL);
}

