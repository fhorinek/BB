/*
 * protocol_def.h
 *
 *  Created on: Dec 4, 2020
 *      Author: horinek
 */

#ifndef DRIVERS_ESP_PROTOCOL_DEF_H_
#define DRIVERS_ESP_PROTOCOL_DEF_H_

typedef uint8_t proto_mac_t[6];

typedef enum {
    PROTO_WIFI_OPEN = 0,         /**< authenticate mode : open */
	PROTO_WIFI_WEP,              /**< authenticate mode : WEP */
	PROTO_WIFI_WPA_PSK,          /**< authenticate mode : WPA_PSK */
	PROTO_WIFI_WPA2_PSK,         /**< authenticate mode : WPA2_PSK */
	PROTO_WIFI_WPA_WPA2_PSK,     /**< authenticate mode : WPA_WPA2_PSK */
	PROTO_WIFI_WPA2_ENTERPRISE,  /**< authenticate mode : WPA2_ENTERPRISE */
	PROTO_WIFI_WPA3_PSK,         /**< authenticate mode : WPA3_PSK */
	PROTO_WIFI_WPA2_WPA3_PSK,    /**< authenticate mode : WPA2_WPA3_PSK */
	PROTO_WIFI_WAPI_PSK,         /**< authenticate mode : WAPI_PSK */
} proto_security_t;

// type FROM STM                    FROM ESP
// 0x00 -                           Debug messages
// 0x01 PING                        PONG
// 0x02 Version request             Version

#define PROTO_DEBUG             0x00
#define PROTO_NA                0xFF


#define PROTO_PING              0x01
#define PROTO_PONG              0x01

#define PROTO_ID_STR_LEN        10
#define PROTO_FW_STR_LEN        16
#define PROTO_HW_STR_LEN       4

#define PROTO_STM_INFO       	0x02
typedef struct {
	char id[PROTO_ID_STR_LEN];
	char fw[PROTO_FW_STR_LEN];
	char hw[PROTO_HW_STR_LEN];
} proto_stm_info_t;


#define PROTO_ESP_INFO       0x02
typedef struct {
    proto_mac_t wifi_ap_mac;
    uint8_t amp_ok;
	uint8_t server_ok;
    proto_mac_t wifi_sta_mac;
    uint8_t _pad_2[2];
    proto_mac_t bluetooth_mac;
} proto_esp_info_t;

#define PROTO_SPI_PREPARE       0x03
typedef struct {
    uint32_t data_lenght;
} proto_spi_prepare_t;

#define PROTO_SPI_READY         0x03
typedef struct {
    uint32_t data_lenght;
} proto_spi_ready_t;

#define PROTO_SPI_CANCEL        0x04


#define PROTO_FS_PATH_LEN       128
#define PROTO_FS_NAME_LEN       32

//-------------------- Sound --------------------

#define PROTO_SET_VOLUME        0x10

#define PROTO_VOLUME_MASTER    	0
#define PROTO_VOLUME_VARIO     	1
#define PROTO_VOLUME_SOUND     	2
#define PROTO_VOLUME_A2DP      	3

typedef struct {
    uint8_t type;
    uint8_t val;
} proto_volume_t;


#define PROTO_SOUND_START		0x11
#define PROTO_SOUND_REQ_MORE	0x11

#define PROTO_FILE_WAV			0
#define PROTO_FILE_AAC			1
#define PROTO_FILE_AMR			2
#define PROTO_FILE_FLAC			3
#define PROTO_FILE_MP3			4
#define PROTO_FILE_OGG			5
#define PROTO_FILE_OPUS			6

typedef struct {
    uint8_t file_id;
    uint8_t file_type;
    uint32_t file_lenght;
} proto_sound_start_t;

typedef struct {
	uint8_t id;
    uint32_t data_lenght;
} proto_sound_req_more_t;

#define PROTO_SOUND_STOP		0x12

#define PROTO_TONE_PLAY         0x13
typedef struct {
    uint16_t freq[8];
    uint16_t dura[8];
    uint8_t size;
    uint8_t id;
} proto_tone_play_t;
#define PROTO_TONE_ACK	        0x14

//-------------------- WiFi --------------------

#define PROTO_WIFI_MODE_OFF     0
#define PROTO_WIFI_MODE_ON      1
#define PROTO_WIFI_MODE_ACTIVE  2

#define PROTO_WIFI_SSID_LEN   	32
#define PROTO_WIFI_PASS_LEN		64

#define PROTO_WIFI_SET_MODE     0x20
#define PROTO_WIFI_MODE         0x20

typedef struct {
    char ssid[PROTO_WIFI_SSID_LEN];
    char pass[PROTO_WIFI_PASS_LEN];
    uint8_t client;
    uint8_t ap;
} proto_wifi_mode_t;

#define PROTO_WIFI_SCAN_START   0x21
#define PROTO_WIFI_SCAN_RES     0x21
typedef struct {
    char name[PROTO_WIFI_SSID_LEN];
    proto_mac_t mac;
    int8_t rssi;
    uint8_t security;
    uint8_t ch;
} proto_wifi_scan_res_t;

#define PROTO_WIFI_SCAN_STOP    0x22
#define PROTO_WIFI_SCAN_END     0x22

#define PROTO_WIFI_CONNECT      0x23
#define PROTO_WIFI_CONNECTED    0x23

typedef struct {
    char ssid[PROTO_WIFI_SSID_LEN];
    char pass[PROTO_WIFI_PASS_LEN];
    uint8_t mac[6];
    uint8_t ch;
} proto_wifi_connect_t;

typedef struct {
    char ssid[PROTO_WIFI_SSID_LEN];
    char pass[PROTO_WIFI_PASS_LEN];
} proto_wifi_connected_t;

#define PROTO_WIFI_DISCONNECT	   	0x24
#define PROTO_WIFI_DISCONNECTED    	0x24

#define PROTO_WIFI_SET_IP			0x25
#define PROTO_WIFI_GOT_IP			0x25

typedef struct {
	uint8_t ip[4];
	uint8_t mask[4];
	uint8_t gw[4];
	uint8_t dhcp[4];
} proto_wifi_got_ip_t;


#define PROTO_WIFI_ENABLED      	0x26
#define PROTO_WIFI_DISABLED      	0x27

#define PROTO_WIFI_AP_ENABLED       0x28
typedef struct {
	uint8_t ip[4];
} proto_wifi_ap_enabled_t;

#define PROTO_WIFI_AP_DISABLED      0x29

#define PROTO_WIFI_AP_CONNETED	    0x2A
typedef struct {
	uint8_t mac[6];
} proto_wifi_client_connected_t;

#define PROTO_WIFI_AP_DISCONNETED   0x2B

#define PROTO_WIFI_RSSI				0x2D
typedef struct {
    int8_t rssi;
} proto_wifi_rssi_t;

#define PROTO_URL_LEN   128

//-------------------- WiFi Download --------------------

#define PROTO_DOWNLOAD_URL          0x30
#define PROTO_DOWNLOAD_INFO         0x30

typedef struct {
    char url[PROTO_URL_LEN];
    uint8_t data_id;
} proto_download_url_t;

#define PROTO_DOWNLOAD_OK               0
#define PROTO_DOWNLOAD_DONE             1
#define PROTO_DOWNLOAD_NOT_FOUND        10
#define PROTO_DOWNLOAD_NO_CONNECTION    20
#define PROTO_DOWNLOAD_TIMEOUT		    30
#define PROTO_DOWNLAOD_NO_FREE_SLOT     0xFF

typedef struct {
    uint32_t size;
    uint8_t result;
    uint8_t end_point;
} proto_download_info_t;

#define PROTO_DOWNLOAD_STOP         0x31

typedef struct {
    uint8_t data_id;
} proto_download_stop_t;

//-------------------- WiFi Upload --------------------

#define PROTO_UPLOAD_URL          0x38
#define PROTO_UPLOAD_INFO         0x38

typedef struct {
    uint8_t data_id;
    char url[PROTO_URL_LEN];
    char file_path[PROTO_FS_PATH_LEN];
    uint32_t file_size;
} proto_upload_request_t;

#define PROTO_UPLOAD_IN_PROGRESS        0
#define PROTO_UPLOAD_DONE               1
#define PROTO_UPLOAD_FAILED             10
#define PROTO_UPLOAD_NO_CONNECTION      20

typedef struct {
    uint8_t data_id;
    uint8_t status;
    uint32_t transmitted_size;
} proto_upload_info_t;

#define PROTO_UPLOAD_STOP         0x39

typedef struct {
    uint8_t data_id;
} proto_upload_stop_t;

//-------------------- Filesystem --------------------

#define PROTO_FS_LIST_REQ			0x40
#define PROTO_FS_LIST_RES			0x40

#define PROTO_FS_TYPE_FILE			0b00000001
#define PROTO_FS_TYPE_FOLDER		0b00000010
#define PROTO_FS_TYPE_END			0b10000000

typedef struct {
	char path[PROTO_FS_PATH_LEN];
	uint8_t filter;
	uint8_t req_id;
} proto_fs_list_req_t;

typedef struct {
	char name[PROTO_FS_NAME_LEN];
	uint32_t size;
	uint8_t type;
	uint8_t req_id;
} proto_fs_list_res_t;

#define PROTO_FS_GET_FILE_REQ		0x41
#define PROTO_FS_GET_FILE_RES		0x41

typedef struct {
	char path[PROTO_FS_PATH_LEN];
	uint16_t chunk_size;
	uint8_t req_id;
} proto_fs_get_file_req_t;

typedef struct {
	uint8_t req_id;
} proto_fs_get_file_res_t;

#define PROTO_FS_SAVE_FILE_REQ		0x42
typedef struct {
	char path[PROTO_FS_PATH_LEN];
	uint32_t size;
	uint8_t req_id;
} proto_fs_save_file_req_t;

#define PROTO_FS_DELETE_FILE_REQ	0x43
typedef struct {
	char path[PROTO_FS_PATH_LEN];
} proto_fs_delete_file_req_t;

#define PROTO_FS_RENAME_FILE_REQ	0x44
typedef struct {
	char old_path[PROTO_FS_PATH_LEN];
	char new_path[PROTO_FS_PATH_LEN];
} proto_fs_rename_file_req_t;


//response trought SPI

//-------------------- Bluetooth --------------------

#define PROTO_BT_SET_MODE       	0x50

#define PROTO_BT_NAME_LEN		16
#define PROTO_BT_PIN_LEN		8

typedef struct {
    char name[PROTO_BT_NAME_LEN];
    char pin[PROTO_BT_PIN_LEN];
    proto_mac_t a2dp_autoconnect;

    bool enabled;
    bool a2dp;
    bool spp;
    bool ble;

} proto_set_bt_mode_t;

#define PROTO_BT_MODE       	0x50
typedef struct {
	bool enabled;
} proto_bt_mode_t;

#define PROTO_BT_DISCOVERABLE  	0x51
typedef struct {
	bool enabled;
} proto_bt_discoverable_t;

#define PROTO_BT_PAIR_REQ       0x52
typedef struct {
    proto_mac_t dev;
    uint32_t value;
    bool cancel;
    bool ble;
    bool only_show;
} proto_bt_pair_req_t;

#define PROTO_BT_PAIR_RES       0x52
typedef struct {
    proto_mac_t dev;
    bool pair;
    bool ble;
} proto_bt_pair_res_t;

#define PROTO_BT_NOTIFY         0x53

#define PROTO_BT_DEV_NAME_LEN       32
#define PROTO_BT_MODE_PAIRED        0b00000001
#define PROTO_BT_MODE_CONNECTED     0b00000010
#define PROTO_BT_MODE_DISCONNECTED  0b00000100
#define PROTO_BT_MODE_A2DP          0b00001000
#define PROTO_BT_MODE_SPP           0b00010000
#define PROTO_BT_MODE_BLE           0b00100000

#define PROTO_BT_UNPAIR         0x54


typedef struct {
    proto_mac_t dev;
    char dev_name[PROTO_BT_DEV_NAME_LEN];
    uint8_t mode;
} proto_bt_notify_t;


//-------------------- Bluetooth Telemetry --------------------

#define PROTO_TELE_SEND			0x60
#define PROTO_TELE_RECV			0x60
#define PROTO_TELE_BUFF_LEN		128

typedef struct {
    char message[PROTO_TELE_BUFF_LEN];
    uint8_t len;
} proto_tele_send_t;

typedef struct {
    char message[PROTO_TELE_BUFF_LEN];
    uint8_t len;
} proto_tele_recv_t;

#define PROTO_TELE_SEND_ACK		0x61

//-------------------- Fanet & Tasks --------------------

#define PROTO_FANET_BOOT0_CTRL			0xF0
typedef struct {
	bool level; //high output or input
} proto_fanet_boot0_ctrl_t;

#define PROTO_FAKE_GNSS					0xF1
typedef struct {
	int32_t timestamp;
	int32_t lat;
	int32_t lon;
	int16_t speed;
	int16_t heading;
	uint8_t fix; //0: No signal, 2: 2D, 3: 3D, 0xFF turn off fake gnss
} proto_fake_gnss_t;

#define PROTO_FAKE_GNSS_ACK             0xF1


#define PROTO_GET_TASKS                 0xF2
#define PROTO_TASKS_RES                 0xF2

typedef struct {
    uint32_t total_time;
    uint8_t number_of_tasks;
    uint8_t _pad[3];
} proto_tasks_head_t;

#define PROTO_TASK_NAME_LEN     32

typedef struct {
    uint32_t run_time;
    char name[PROTO_TASK_NAME_LEN];

    uint16_t watermark;
    uint8_t number;
    uint8_t priority: 6;
    uint8_t core: 2;
} proto_tasks_item_t;


#define PROTO_RESET_NVM         		0xF3
#define PROTO_RESET_NVM_ACK       		0xF4

//-------------------------------------------


#define SPI_BUFFER_SIZE (1024 * 4)

typedef struct
{
    uint8_t packet_type;
    uint8_t data_id;
    uint16_t data_len;
} proto_spi_header_t;

#define SPI_EP_SOUND		0
#define SPI_EP_MUSIC		1
#define SPI_EP_DOWNLOAD     2
#define SPI_EP_FILE		    3


#endif /* DRIVERS_ESP_PROTOCOL_DEF_H_ */
