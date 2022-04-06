/*
 * protocol.c
 *
 *  Created on: Dec 4, 2020
 *      Author: horinek
 */

#define DEBUG_LEVEL DBG_DEBUG

#include "protocol.h"
#include "esp.h"
#include "sound/sound.h"

#include "etc/stream.h"
#include "etc/format.h"

#include "gui/statusbar.h"
#include "gui/bluetooth.h"
#include "gui/dbg_overlay.h"
#include "gui/tasks/menu/bluetooth.h"
#include "gui/fw_notify.h"

#include "etc/safe_uart.h"

#include "fc/fc.h"

#include "system/upload_crash.h"

#include "file.h"

static bool spi_prepare_in_progress = false;

safe_uart_t protocol_tx;

void esp_send_ping()
{
    protocol_send(PROTO_PING, NULL, 0);
}

void esp_get_version()
{
    protocol_send(PROTO_PING, NULL, 0);
}

void esp_set_volume(uint8_t ch, uint8_t vol)
{
    proto_volume_t data;
    data.type = ch;
    data.val = vol;

    protocol_send(PROTO_SET_VOLUME, (void *) &data, sizeof(data));
}

void esp_boot0_ctrl(bool val)
{
    proto_fanet_boot0_ctrl_t data;
    data.level = val;

    protocol_send(PROTO_FANET_BOOT0_CTRL, (void *) &data, sizeof(data));
}


void esp_spi_prepare()
{
	if (!spi_prepare_in_progress)
	{
		protocol_send(PROTO_SPI_PREPARE, NULL, 0);
		spi_prepare_in_progress = true;
	}
}

void esp_sound_start(uint8_t id, uint8_t type, uint32_t size)
{
    proto_sound_start_t data;
    data.file_id = id;
    data.file_type = type;
    data.file_lenght = size;

    protocol_send(PROTO_SOUND_START, (void *) &data, sizeof(data));
}

void esp_sound_stop()
{
    protocol_send(PROTO_SOUND_STOP, NULL, 0);
}

void esp_set_wifi_mode()
{
    proto_wifi_mode_t data;
    data.client = config_get_bool(&profile.wifi.enabled) ? PROTO_WIFI_MODE_ON : PROTO_WIFI_MODE_OFF;
    data.ap = config_get_bool(&profile.wifi.ap) ? PROTO_WIFI_MODE_ON : PROTO_WIFI_MODE_OFF;

    if (data.ap)
    {
        strcpy(data.ssid, config_get_text(&config.device_name));
        strcpy(data.pass, config_get_text(&config.wifi.ap_pass));
    }

    protocol_send(PROTO_WIFI_SET_MODE, (void *) &data, sizeof(data));
}

bool esp_wifi_scanning()
{
    return fc.esp.wifi_list_cb != NULL;
}

void esp_wifi_start_scan(wifi_list_update_cb cb)
{
    fc.esp.wifi_list_cb = cb;

    protocol_send(PROTO_WIFI_SCAN_START, NULL, 0);
}

void esp_wifi_stop_scan()
{
    if (fc.esp.wifi_list_cb != NULL)
    {
    	fc.esp.wifi_list_cb = NULL;
    	protocol_send(PROTO_WIFI_SCAN_STOP, NULL, 0);
    }

}

void esp_wifi_connect(uint8_t mac[6], char * ssid, char * pass, uint8_t ch)
{
    __align proto_wifi_connect_t data;

    memcpy(data.mac, mac, 6);
    data.ch = ch;
    strncpy(data.ssid, ssid, PROTO_WIFI_SSID_LEN);
    strncpy(data.pass, pass, PROTO_WIFI_PASS_LEN);

    protocol_send(PROTO_WIFI_CONNECT, (void *) &data, sizeof(data));
}

void esp_set_bt_mode()
{
    proto_set_bt_mode_t data;
    INFO("esp_set_bt_mode");
    data.enabled = config_get_bool(&profile.bluetooth.enabled);
    data.a2dp = config_get_bool(&profile.bluetooth.a2dp);
    data.spp = config_get_bool(&profile.bluetooth.spp);
    data.ble = config_get_bool(&profile.bluetooth.ble);
    strncpy(data.name, config_get_text(&config.device_name), PROTO_BT_NAME_LEN);
    strncpy(data.pin, config_get_text(&config.bluetooth.pin), PROTO_BT_PIN_LEN);

    protocol_send(PROTO_BT_SET_MODE, (void *) &data, sizeof(data));
}

void esp_configure()
{
    //reset state
    esp_state_reset();

    //set volume
    esp_set_volume(PROTO_VOLUME_MASTER, config_get_int(&profile.audio.master_volume));
    esp_set_volume(PROTO_VOLUME_VARIO, config_get_int(&profile.audio.vario_volume));
    esp_set_volume(PROTO_VOLUME_A2DP, config_get_int(&profile.audio.a2dp_volume));
    esp_set_volume(PROTO_VOLUME_SOUND, config_get_int(&profile.audio.sound_volume));

    //set wifi mode
    esp_set_wifi_mode();

    //set bt mode
    esp_set_bt_mode();

    //set normal mode
    fc.esp.mode = esp_normal;
}

uint8_t esp_http_get(char * url, uint8_t slot_type, download_slot_cb_t cb)
{
    uint8_t data_id = download_slot_create(slot_type, cb);
    if (data_id == DOWNLOAD_SLOT_NONE)
        return DOWNLOAD_SLOT_NONE;

    proto_download_url_t data;
    strncpy(data.url, url, PROTO_URL_LEN);
    data.data_id = data_id;

    protocol_send(PROTO_DOWNLOAD_URL, (void *) &data, sizeof(data));

    return data_id;
}

void esp_http_stop(uint8_t data_id)
{
    proto_download_stop_t data;
    data.data_id = data_id;

    download_slot_cancel(data_id);

    protocol_send(PROTO_DOWNLOAD_STOP, (void *) &data, sizeof(data));
}

uint8_t esp_http_post(char * url, char * file_path, upload_slot_callback_t callback)
{
    uint8_t data_id = upload_slot_create(callback);
    if (data_id == UPLOAD_SLOT_NONE)
        return UPLOAD_SLOT_NONE;

    FILINFO file_info;
    FRESULT res = f_stat(file_path, &file_info);
    if (res != FR_OK)
    {
        return UPLOAD_SLOT_NO_FILE;
    }

    proto_upload_request_t upload_request;
    strncpy(upload_request.url, url, PROTO_URL_LEN);
    strncpy(upload_request.file_path, file_path, PROTO_FS_PATH_LEN);
    upload_request.file_size = file_info.fsize;
    upload_request.data_id = data_id;

    DBG("Upload: %s (size: %ld)", file_path, file_info.fsize);

    protocol_send(PROTO_UPLOAD_URL, (void *) &upload_request, sizeof(upload_request));

    return data_id;
}

void esp_http_post_stop(uint8_t data_id)
{
    proto_upload_stop_t stop;
    stop.data_id = data_id;

    upload_slot_cancel(data_id);

    protocol_send(PROTO_UPLOAD_STOP, (void *) &stop, sizeof(stop));
}


void protocol_init()
{
    su_init(&protocol_tx, esp_uart, 1024, 128);
}

void protocol_send(uint8_t type, uint8_t * data, uint16_t data_len)
{
	if (fc.esp.mode == esp_off || fc.esp.mode == esp_external_auto || fc.esp.mode == esp_external_manual)
		return;

    uint8_t buf_out[data_len + STREAM_OVERHEAD];

    stream_packet(type, buf_out, data, data_len);

    su_write(&protocol_tx, buf_out, sizeof(buf_out));
}

void protocol_handle(uint8_t type, uint8_t * data, uint16_t len)
{
//    if (type != PROTO_DEBUG)
//        DBG("protocol_handle %u", type);

	fc.esp.last_ping = HAL_GetTick();

    switch(type)
    {
        case(PROTO_DEBUG): //Debug output
        {
            uint8_t level = data[0];
            if (level == 0xFF)
                level = DBG_INFO;

            data[len] = 0;

            char buff[len + 4];
            sprintf(buff, "\t%s", data + 1);

            debug_send(level, (char *)buff);
        }
        break;

        case(PROTO_PONG):
        {
        	//ping - pong
        }
        break;

        case(PROTO_DEVICE_INFO):
        {
            proto_device_info_t * packet = (proto_device_info_t *)data;

            char tmp[20];
            format_mac(tmp, packet->bluetooth_mac);
            DBG("ESP bt: %s", tmp);
            format_mac(tmp, packet->wifi_ap_mac);
            DBG("ESP ap: %s", tmp);
            format_mac(tmp, packet->wifi_sta_mac);
            DBG("ESP sta: %s", tmp);

            fc.esp.amp_status = packet->amp_ok ? fc_dev_ready : fc_dev_error;
            fc.esp.server_status = packet->server_ok ? fc_dev_ready : fc_dev_error;

            esp_configure();

            safe_memcpy(fc.esp.mac_ap, packet->wifi_ap_mac, 6);
            safe_memcpy(fc.esp.mac_sta, packet->wifi_sta_mac, 6);
            safe_memcpy(fc.esp.mac_bt, packet->bluetooth_mac, 6);

            //enable tone
            fc.esp.tone_next_tx = 0;
        }
        break;

        case(PROTO_SPI_READY):
        {
            spi_start_transfer(((proto_spi_ready_t *)data)->data_lenght);
            spi_prepare_in_progress = false;
        }
        break;

        case(PROTO_SOUND_REQ_MORE):
        {
            proto_sound_req_more_t * packet = (proto_sound_req_more_t *)data;
            sound_read_next(packet->id, packet->data_lenght);
        }
        break;

        case(PROTO_WIFI_SCAN_RES):
        {
            proto_wifi_scan_res_t * packet = (proto_wifi_scan_res_t *)data;
            if (fc.esp.wifi_list_cb != NULL)
            {
                fc.esp.wifi_list_cb(packet);
            }
        }
        break;

        case(PROTO_TONE_ACK):
		{
//			DBG("Tone ready");
        	fc.esp.tone_next_tx = 0;
		}
        break;

        case(PROTO_WIFI_SCAN_END):
            if (fc.esp.wifi_list_cb != NULL)
            {
                fc.esp.wifi_list_cb(NULL);
                fc.esp.wifi_list_cb = NULL;
            }
        break;

        case(PROTO_WIFI_ENABLED):
            fc.esp.state |= ESP_STATE_WIFI_CLIENT;
        break;

        case(PROTO_WIFI_DISABLED):
            fc.esp.state &= ~ESP_STATE_WIFI_CLIENT;
        break;

        case(PROTO_WIFI_CONNECTED):
        {
            char msg[PROTO_WIFI_SSID_LEN + 32];

            proto_wifi_connected_t * packet = (proto_wifi_connected_t *)data;
            db_insert(PATH_NETWORK_DB, packet->ssid, packet->pass);
            sprintf(msg, "Connected to '%s'", packet->ssid);
            statusbar_msg_add(STATUSBAR_MSG_INFO, msg);
            strncpy(fc.esp.ssid, packet->ssid, PROTO_WIFI_SSID_LEN);
            fc.esp.state |= ESP_STATE_WIFI_CONNECTED;

            if (config_get_bool(&config.system.check_for_updates))
            {
                osTimerId_t timer = osTimerNew(check_for_update, osTimerOnce, NULL, NULL);
                osTimerStart(timer, 5000);
            }

            upload_crash_reports_schedule();
        }
        break;

        case(PROTO_WIFI_DISCONNECTED):
            memset(fc.esp.ip_sta, 0, 4);

            if (fc.esp.state & ESP_STATE_WIFI_CONNECTED)
            {
                statusbar_msg_add(STATUSBAR_MSG_INFO, "Wi-Fi disconnected");
            }
            else
            {
                statusbar_msg_add(STATUSBAR_MSG_ERROR, "Unable to connect");
            }

            fc.esp.state &= ~ESP_STATE_WIFI_CONNECTED;

        break;

        case(PROTO_WIFI_GOT_IP):
        {
            proto_wifi_got_ip_t * packet = (proto_wifi_got_ip_t *)data;
            safe_memcpy(fc.esp.ip_sta, packet->ip, 4);
        }
        break;

        case(PROTO_WIFI_AP_ENABLED):
        {
            proto_wifi_ap_enabled_t * packet = (proto_wifi_ap_enabled_t *)data;
            safe_memcpy(fc.esp.ip_ap, packet->ip, 4);
            fc.esp.state |= ESP_STATE_WIFI_AP;
        }
        break;

        case(PROTO_WIFI_AP_DISABLED):
            //reset ip
            memset(fc.esp.ip_ap, 0, 4);
            fc.esp.state &= ~ESP_STATE_WIFI_AP;
        break;

        case(PROTO_WIFI_AP_CONNETED):
        {
            char * msg = "Device connected to AP";
            statusbar_msg_add(STATUSBAR_MSG_INFO, msg);
            fc.esp.state |= ESP_STATE_WIFI_AP_CONNECTED;
        }
        break;

        case(PROTO_WIFI_AP_DISCONNETED):
            fc.esp.state &= ~ESP_STATE_WIFI_AP_CONNECTED;
        break;

        case(PROTO_DOWNLOAD_INFO):
            download_slot_process_info((proto_download_info_t *)data);
        break;

        case(PROTO_UPLOAD_INFO):
            upload_slot_process_info((proto_upload_info_t *)data);
        break;

        case(PROTO_FS_LIST_REQ):
        	file_list_path((proto_fs_list_req_t *)data);
        break;

        case(PROTO_FS_GET_FILE_REQ):
		{
			//create new thread
			proto_fs_get_file_req_t * packet = (proto_fs_get_file_req_t *) malloc(sizeof(proto_fs_get_file_req_t));
			safe_memcpy(packet, data, sizeof(proto_fs_get_file_req_t));

			xTaskCreate((TaskFunction_t)file_send_file, "file_send_file", 1024 * 4, (void *)packet, 24, NULL);
		}
		break;

        case(PROTO_FS_SAVE_FILE_REQ):
			file_get_file_info((proto_fs_save_file_req_t *) data);
		break;

        case(PROTO_BT_MODE):
		{
        	proto_bt_mode_t * packet = (proto_bt_mode_t *)data;
        	if (packet->enabled)
        	{
        		fc.esp.state |= ESP_STATE_BT_ON;

        	    //if inside bluetooth menu, make bt discoverable
        	    if (config_get_bool(&profile.bluetooth.enabled) && gui.task.actual == &gui_bluetooth)
        	    {
        	    	bluetooth_discoverable(true);
        	    }
        	}
        	else
        	{
        		fc.esp.state &= ~ESP_STATE_BT_ON;
        	}
		}
		break;

        case(PROTO_BT_PAIR_REQ):
		{
        	bluetooth_pari_req((proto_bt_pair_req_t *)data);
		}
        break;

        case(PROTO_BT_NOTIFY):
		{
        	bluetooth_notify((proto_bt_notify_t *)data);
		}
        break;

        case(PROTO_TELE_SEND_ACK):
			//telemety packet was send3
        break;

        case(PROTO_FAKE_GNSS):
		{
        	proto_fake_gnss_t * packet = (proto_fake_gnss_t *)data;

			if (config_get_bool(&config.time.sync_gnss) && packet->timestamp != 0)
			{
				fc_set_time_from_utc(packet->timestamp);
				fc.gnss.utc_time = packet->timestamp;
			}
			else
			{
				fc.gnss.utc_time = fc_get_utc_time();
			}


        	FC_ATOMIC_ACCESS
			{
        		fc.gnss.itow = HAL_GetTick();
				fc.gnss.fake = true;
				fc.gnss.latitude = packet->lat;
				fc.gnss.longtitude = packet->lon;
				fc.gnss.heading = packet->heading;
				fc.gnss.ground_speed = packet->speed;
				fc.gnss.fix = packet->fix;
				fc.gnss.new_sample = 0xFF;
			}

    		config_set_big_int(&profile.ui.last_lat, fc.gnss.latitude);
    		config_set_big_int(&profile.ui.last_lon, fc.gnss.longtitude);
		}
        break;

        case(PROTO_TASKS_RES):
        {
            uint8_t * packet = (uint8_t *) malloc(len);
            safe_memcpy(packet, data, len);
            xTaskCreate((TaskFunction_t)dbg_overlay_tasks_update, "dbg_overlay_update", 1024 * 2, (void *)packet, 24, NULL);
        }
        break;

        default:
            DBG("Unknown packet: %02X", type);
            DUMP(data, len);
    }
}

