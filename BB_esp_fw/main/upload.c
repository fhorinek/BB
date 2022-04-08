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

#define UPLOAD_FS_WAIT          500
#define UPLOAD_FS_CHUNK_SIZE    1024

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

char* map_content_type(uint8_t content_type)
{
    switch (content_type)
    {
        case (PROTO_UPLOAD_CONTENT_TYPE_TEXT):
            return "text/plain";
        case (PROTO_UPLOAD_CONTENT_TYPE_BINARY):
            return "application/octet-stream";
        case (PROTO_UPLOAD_CONTENT_TYPE_ZIP):
            return "application/zip";
    }

    ERR("Upload: Unexpected content type");
    ASSERT(false);
    return "";
}

uint32_t upload_transmit_chunks(uint8_t data_id, esp_http_client_handle_t http_client, ll_item_t *file_handle)
{
    uint32_t transmitted_size = 0;
    uint32_t received_chunk_size = 0;

    proto_upload_info_t info = {
            .data_id = data_id,
            .status = PROTO_UPLOAD_IN_PROGRESS
    };

    while (true)
    {
        if (!xSemaphoreTake(file_handle->read, UPLOAD_FS_WAIT))
        {
            ERR("Upload: File system operation timeout!");
            return transmitted_size;
        }

        if (file_handle->data_ptr == NULL)
        {
            ERR("Upload: File not found");
            return transmitted_size;
        }

        received_chunk_size = *((uint32_t*) file_handle->data_ptr);
        int write_size = esp_http_client_write(http_client, file_handle->data_ptr + 4, received_chunk_size);
        transmitted_size += write_size;

        free(file_handle->data_ptr);
        file_handle->data_ptr = NULL;

        if (write_size != received_chunk_size)
        {
            ERR("Upload: Failed to transmit all data %u/%u", write_size, received_chunk_size);
            return transmitted_size;
        }

        xSemaphoreGive(file_handle->write);

        if (received_chunk_size < UPLOAD_FS_CHUNK_SIZE)
        {
            return transmitted_size; // Transmission complete
        }

        // Send progress
        info.transmitted_size = transmitted_size;
        protocol_send(PROTO_UPLOAD_INFO, (void*) &info, sizeof(info));

        if (upload_is_canceled(data_id))
        {
            return transmitted_size;
        }
    }
}

uint32_t upload_transmit(proto_upload_request_t *upload_request, esp_http_client_handle_t http_client)
{
    ll_item_t *file_handle = ll_add_item(PROTO_FS_GET_FILE_RES, upload_request->data_id, 0);

    proto_fs_get_file_req_t file_request = {
            .req_id = upload_request->data_id,
            .chunk_size = UPLOAD_FS_CHUNK_SIZE
    };
    strncpy(file_request.path, upload_request->file_path, PROTO_FS_PATH_LEN);
    protocol_send(PROTO_FS_GET_FILE_REQ, (void*) &file_request, sizeof(file_request));

    uint32_t transmitted_size = upload_transmit_chunks(upload_request->data_id, http_client, file_handle);

    ll_delete_item(file_handle);

    return transmitted_size;
}

void upload_process_request(proto_upload_request_t *upload_request)
{
    proto_upload_info_t info = {
            .data_id = upload_request->data_id,
            .transmitted_size = 0
    };

    esp_http_client_config_t config = {
            .url = upload_request->url,
            .method = HTTP_METHOD_POST
    };

    esp_http_client_handle_t http_client = esp_http_client_init(&config);
    esp_http_client_set_header(http_client, "Content-Type", map_content_type(upload_request->content_type));

    esp_err_t open_status = esp_http_client_open(http_client, upload_request->file_size);

    if (open_status == ESP_OK)
    {
        info.transmitted_size = upload_transmit(upload_request, http_client);

        if (info.transmitted_size == upload_request->file_size)
        {
            int content_length = esp_http_client_fetch_headers(http_client);
            if (content_length == upload_request->file_size)
            {
                uint16_t status = esp_http_client_get_status_code(http_client);
                if (status == 200) // OK
                {
                    info.status = PROTO_UPLOAD_DONE;
                }
                else
                {
                    info.status = PROTO_UPLOAD_FAILED;
                    WARN("Upload: Failed with status code %u", status);
                }
            }
            else
            {
                info.status = PROTO_UPLOAD_FAILED;
                WARN("Upload: Content length does not match file size %u/%u", content_length, upload_request->file_size);
            }
        }
        else
        {
            info.status = PROTO_UPLOAD_FAILED;
            WARN("Upload: Failed to transmit complete file %u/%u", info.transmitted_size, upload_request->file_size);
        }
    }
    else
    {
        info.status = PROTO_UPLOAD_NO_CONNECTION;
        WARN("Upload: Failed to open connection: %s", esp_err_to_name(open_status));
    }

    protocol_send(PROTO_UPLOAD_INFO, (void*) &info, sizeof(info));

    esp_http_client_close(http_client);
    esp_http_client_cleanup(http_client);

    INFO("Upload: done");

}

void upload_file(proto_upload_request_t *packet)
{
    INFO("Upload: start (%s) ", packet->url);

    upload_start(packet->data_id);

    if (wifi_is_connected())
    {
        upload_process_request(packet);
    }
    else
    {
        proto_upload_info_t info = {
                .data_id = packet->data_id,
                .status = PROTO_UPLOAD_NO_CONNECTION,
                .transmitted_size = 0
        };
        protocol_send(PROTO_UPLOAD_INFO, (void*) &info, sizeof(info));

        WARN("Upload: failed, no WIFI connection");
    }

    upload_stop(packet->data_id);

    free(packet);

    vTaskDelete(NULL);
}

