/*
 * protocol.c
 *
 *  Created on: 4. 12. 2020
 *      Author: horinek
 */

#define DEBUG_LEVEL DGB_DEBUG

#include "protocol.h"
#include "wifi.h"
#include "drivers/uart.h"
#include "etc/stream.h"
#include "drivers/tas5720.h"
#include "drivers/spi.h"
#include "pipeline/sound.h"
#include "pipeline/vario.h"
#include "download.h"
#include "bluetooth/bluetooth.h"

#include "linked_list.h"

static bool protocol_enable_processing = false;

void protocol_enable()
{
	protocol_enable_processing = true;
}

void protocol_send_heartbeat()
{
	protocol_send(PROTO_PING, NULL, 0);
}

void protocol_send_info()
{
    DBG("sending info");
    proto_device_info_t data;
    esp_read_mac((uint8_t *)&data.wifi_sta_mac, ESP_MAC_WIFI_STA);
    esp_read_mac((uint8_t *)&data.wifi_ap_mac, ESP_MAC_WIFI_SOFTAP);
    esp_read_mac((uint8_t *)&data.bluetooth_mac, ESP_MAC_BT);

    data.server_ok = system_status.server_ok;
    data.amp_ok = system_status.amp_ok;

    protocol_send(PROTO_DEVICE_INFO, (uint8_t *)&data, sizeof(data));
}

void protocol_send_spi_ready(uint32_t len)
{
    proto_spi_ready_t data;
    data.data_lenght = len;
    protocol_send(PROTO_SPI_READY, (uint8_t*) &data, sizeof(data));
}

void protocol_send_sound_reg_more(uint8_t id, uint32_t len)
{
	proto_sound_req_more_t data;
	data.id = id;
    data.data_lenght = len;
    protocol_send(PROTO_SOUND_REQ_MORE, (uint8_t*) &data, sizeof(data));
}

//void esp_log_impl_lock(void);
//void esp_log_impl_unlock(void);

void protocol_send(uint8_t type, uint8_t *data, uint16_t data_len)
{
    uint8_t buf_out[data_len + STREAM_OVERHEAD];

    stream_packet(type, buf_out, data, data_len);

    uart_send(buf_out, sizeof(buf_out));
}

void protocol_task_info(void * param)
{
    uint8_t cnt = uxTaskGetNumberOfTasks();
    uint32_t total_time;

    TaskStatus_t * task_status = (TaskStatus_t *) ps_malloc(cnt * sizeof(TaskStatus_t));

    cnt = uxTaskGetSystemState(task_status, cnt, &total_time);

    uint16_t buff_size = sizeof(proto_tasks_head_t) + sizeof(proto_tasks_item_t) * cnt;
    uint8_t * proto_buff = ps_malloc(buff_size);

    proto_tasks_head_t * head = (proto_tasks_head_t *)proto_buff;
    head->number_of_tasks = cnt;

    if( total_time > 0UL )
    {
        for(uint8_t i = 0; i < cnt; i++ )
        {
            TaskStatus_t * ts = task_status + i;
            proto_tasks_item_t * item = (proto_tasks_item_t *)(proto_buff + sizeof(proto_tasks_head_t) + sizeof(proto_tasks_item_t) * i);
            item->run_time = ts->ulRunTimeCounter * 2;
            strncpy(item->name, ts->pcTaskName, PROTO_TASK_NAME_LEN);
            item->watermark = ts->usStackHighWaterMark;
            item->number = ts->xTaskNumber;
            item->priority = ts->uxCurrentPriority;

            if (ts->xCoreID == 0)
                item->core = 0;
            else if (ts->xCoreID == 1)
                item->core = 1;
            else
                item->core = 2;
        }
    }

    protocol_send(PROTO_TASKS_RES, proto_buff, buff_size);

    free(proto_buff);
    free(task_status);
    vTaskDelete(NULL);
}

#define PROTOCOL_SUBPROCESS_PRIORITY	11

void protocol_handle(uint8_t type, uint8_t *data, uint16_t len)
{
    int64_t start = esp_timer_get_time();
    
    if (!protocol_enable_processing)
    {
    	return;
    }

	switch (type)
    {
        case (PROTO_PING):
            protocol_send(PROTO_PONG, NULL, 0);
        break;

        case (PROTO_GET_INFO):
        {
            protocol_send_info();
        }
        break;

        case (PROTO_SET_VOLUME):
        {
            proto_volume_t * packet = (proto_volume_t *)data;

            if (packet->type == PROTO_VOLUME_MASTER)
                tas_volume(packet->val);
        }
        break;

        case (PROTO_SPI_PREPARE):
        {
            spi_prepare_buffer();
        }
        break;

        case (PROTO_SOUND_START):
		{
        	proto_sound_start_t * packet = (proto_sound_start_t *)data;
        	pipe_sound_start(packet->file_id, packet->file_type, packet->file_lenght);
		}
        break;

        case (PROTO_SOUND_STOP):
        	pipe_sound_stop();
        break;

        case (PROTO_TONE_PLAY):
		{
        	proto_tone_play_t * packet = (proto_tone_play_t *) ps_malloc(sizeof(proto_tone_play_t));
        	memcpy(packet, data, sizeof(proto_tone_play_t));

        	xTaskCreate((TaskFunction_t)vario_proces_packet, "vario_proces_packet", 1024 * 3, (void *)packet, PROTOCOL_SUBPROCESS_PRIORITY, NULL);
		}
        break;

        case (PROTO_WIFI_SET_MODE):
		{
        	proto_wifi_mode_t * packet = (proto_wifi_mode_t *) ps_malloc(sizeof(proto_wifi_mode_t));
        	memcpy(packet, data, sizeof(proto_wifi_mode_t));

        	xTaskCreate((TaskFunction_t)wifi_enable, "wifi_enable", 1024 * 3, (void *)packet, PROTOCOL_SUBPROCESS_PRIORITY, NULL);
		}
        break;

        case (PROTO_WIFI_SCAN_START):
        	xTaskCreate((TaskFunction_t)wifi_start_scan, "wifi_start_scan", 1024 * 3, NULL, PROTOCOL_SUBPROCESS_PRIORITY, NULL);
        break;

        case (PROTO_WIFI_SCAN_STOP):
			//hard stop
			esp_wifi_scan_stop();
        break;

        case (PROTO_WIFI_CONNECT):
		{
        	proto_wifi_connect_t * packet = (proto_wifi_connect_t *) ps_malloc(sizeof(proto_wifi_connect_t));
        	memcpy(packet, data, sizeof(proto_wifi_connect_t));

			xTaskCreate((TaskFunction_t)wifi_connect, "wifi_connect", 1024 * 3, (void *)packet, PROTOCOL_SUBPROCESS_PRIORITY, NULL);
		}
        break;


        case (PROTO_DOWNLOAD_URL):
		{
        	proto_download_url_t * packet = (proto_download_url_t *) ps_malloc(sizeof(proto_download_url_t));
        	memcpy(packet, data, sizeof(proto_download_url_t));

			xTaskCreate((TaskFunction_t)download_url, "download_url", 1024 * 4, (void *)packet, PROTOCOL_SUBPROCESS_PRIORITY, NULL);
		}
        break;

        case (PROTO_DOWNLOAD_STOP):
		{
        	download_stop(((proto_download_stop_t *)data)->data_id);
		}
        break;


        case (PROTO_FS_LIST_RES):
		{
        	proto_fs_list_res_t * packet = (proto_fs_list_res_t *)data;
        	ll_item_t * handle = ll_find_item(PROTO_FS_LIST_RES, packet->req_id);
        	if (handle != NULL)
        	{
        		xSemaphoreTake(handle->write, WAIT_INF);
        		memcpy(handle->data_ptr, data, sizeof(proto_fs_list_res_t));
        		xSemaphoreGive(handle->read);
        	}
		}
        break;


        case (PROTO_FS_GET_FILE_RES):
		{
        	//normal transfer is done via SPI, this means that file was not found
        	proto_fs_list_res_t * packet = (proto_fs_list_res_t *)data;
        	ll_item_t * handle = ll_find_item(PROTO_FS_GET_FILE_RES, packet->req_id);
        	if (handle != NULL)
        	{
        		xSemaphoreTake(handle->write, WAIT_INF);
        		handle->data_ptr = NULL;
        		xSemaphoreGive(handle->read);
        	}
		}
        break;

        case (PROTO_BT_SET_MODE):
		{
			proto_set_bt_mode_t * packet = (proto_set_bt_mode_t *) ps_malloc(sizeof(proto_set_bt_mode_t));
			memcpy(packet, data, sizeof(proto_set_bt_mode_t));

			xTaskCreate((TaskFunction_t)bt_set_mode, "bt_set_mode", 1024 * 3, (void *)packet, PROTOCOL_SUBPROCESS_PRIORITY, NULL);
		}
		break;

        case (PROTO_BT_DISCOVERABLE):
		{
        	proto_bt_discoverable_t * packet = (proto_bt_discoverable_t *)data;
        	bt_set_discoverable(packet);
		}
        break;

        case (PROTO_BT_PAIR_RES):
        {
            proto_bt_pair_res_t * packet = (proto_bt_pair_res_t *)data;
            bt_confirm_pair(packet);
        }
        break;

        case (PROTO_BT_UNPAIR):
        {
            bt_unpair();
        }
        break;

        case (PROTO_TELE_SEND):
        {
            bt_tele_send((proto_tele_send_t *)data);
        }
        break;

        case (PROTO_FANET_BOOT0_CTRL):
		{
        	proto_fanet_boot0_ctrl_t * packet = (proto_fanet_boot0_ctrl_t *)data;
        	fanet_boot0_ctrl(packet);
		}
        break;

        case (PROTO_GET_TASKS):
        {
            xTaskCreate((TaskFunction_t)protocol_task_info, "protocol_task_info", 1024 * 3, NULL, PROTOCOL_SUBPROCESS_PRIORITY, NULL);
        }
        break;

        default:
            DBG("Unknown packet");
            DUMP(data, len);
        break;
    }

	int64_t delta = esp_timer_get_time() - start;
	if (delta > 1000)
	{
		DBG("handle %02X %0.3fms", type, delta / 1000.0);
	}
}
