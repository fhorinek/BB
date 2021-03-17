/*
 * protocol_def.h
 *
 *  Created on: Dec 4, 2020
 *      Author: horinek
 */

#ifndef DRIVERS_ESP_PROTOCOL_DEF_H_
#define DRIVERS_ESP_PROTOCOL_DEF_H_

// type FROM STM                    FROM ESP
// 0x00 -                           Debug messages
// 0x01 PING                        PONG
// 0x02 Version request             Version

#define PROTO_DEBUG             0x00
#define PROTO_NA                0xFF


#define PROTO_PING              0x01
#define PROTO_PONG              0x01

#define PROTO_GET_VERSION       0x02
#define PROTO_VERSION           0x02

typedef struct {
    uint32_t version;
} proto_version_t;

#define PROTO_SET_VOLUME        0x03
#define PROTO_VOLUME            0x03

#define PROTO_VOLUME_MASTER    	0
#define PROTO_VOLUME_VARIO     	1
#define PROTO_VOLUME_SOUND     	2
#define PROTO_VOLUME_A2DP      	3

typedef struct {
    uint8_t type;
    uint8_t val;
} proto_volume_t;



#define PROTO_SPI_PREPARE      	0x04
#define PROTO_SPI_READY        	0x04

typedef struct {
    uint32_t data_lenght;
} proto_spi_ready_t;

#define PROTO_SOUND_START		0x05
#define PROTO_SOUND_REQ_MORE	0x05

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

#define PROTO_SOUND_STOP		0x06

#define PROTO_WIFI_SET_MODE     0x07
#define PROTO_WIFI_MODE         0x07

#define PROTO_WIFI_MODE_OFF     0
#define PROTO_WIFI_MODE_ON      1
#define PROTO_WIFI_MODE_ACTIVE  2

typedef struct {
    uint8_t client;
    uint8_t ap;
} proto_wifi_mode_t;


#define PROTO_WIFI_SCAN_START   0x08
#define PROTO_WIFI_SCAN_RES     0x08
typedef struct {
    uint8_t magic;
    char name[33];
    uint8_t flags;
} proto_wifi_scan_res_t;

#define PROTO_WIFI_SCAN_STOP    0x09

#define PROTO_SET_DEVICE_NAME   0x0A
#define PROTO_DEV_NAME_LEN   16
typedef struct {
    char name[PROTO_DEV_NAME_LEN];
} proto_set_device_name_t;

#define PROTO_BT_SET_MODE       0x0B
typedef struct {
    bool a2dp;
    bool spp;
    bool ble;
    char pin[7];
} proto_set_bt_mode_t;

//-------------------------------------------


#define SPI_BUFFER_SIZE (1024 * 8)

typedef struct
{
    uint8_t packet_type;
    uint8_t data_id;
    uint8_t res[2];
} proto_spi_header_t;

#define SPI_EP_SOUND		0
#define SPI_EP_MUSIC		1


#endif /* DRIVERS_ESP_PROTOCOL_DEF_H_ */
