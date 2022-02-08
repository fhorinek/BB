/*
 * common.h
 *
 *  Created on: 17. 1. 2022
 *      Author: horinek
 */

#ifndef MAP_COMMON_H_
#define MAP_COMMON_H_

//fake strato function here

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "etc/fake_fatfs.h"
#include "config/db.h"

#include "lvgl/lvgl.h"

#define gui_lock_acquire()
#define gui_lock_release()
#define FC_ATOMIC_ACCESS

#define GNSS_MUL				10000000l

typedef union
{
    uint16_t uint16;
    int16_t int16;
    uint8_t uint8[2];
} byte2;

#define __align

#define PATH_CONFIG_DIR		"config"
#define PATH_DEVICE_CFG		PATH_CONFIG_DIR "/device.cfg"
#define PATH_PROFILE_DIR    PATH_CONFIG_DIR "/profiles"
#define PATH_VARIO_DIR      PATH_CONFIG_DIR "/vario"
#define PATH_PILOT_DIR      PATH_CONFIG_DIR "/pilots"
#define PATH_PAGES_DIR      PATH_CONFIG_DIR "/pages"
#define PATH_NETWORK_DB     PATH_CONFIG_DIR "/networks.cfg"

#define PATH_SCREENSHOT     "scrshot"

#define PATH_SYSTEM_DIR     "system"
#define PATH_TEMP_DIR       PATH_SYSTEM_DIR "/temp"
#define PATH_FW_DIR         PATH_SYSTEM_DIR "/fw"
#define PATH_CACHE_DIR      PATH_SYSTEM_DIR "/cache"
#define PATH_COREDUMP       PATH_SYSTEM_DIR "/coredump_stm.bin"
#define PATH_MAP_CACHE_DIR  PATH_CACHE_DIR "/map"

#define PATH_ASSET_DIR      PATH_SYSTEM_DIR "/assets"
#define PATH_DEFAULTS_DIR   PATH_ASSET_DIR "/defaults"
#define PATH_LOGS_DIR       "logs"
#define PATH_NEW_FW         PATH_ASSET_DIR "/NEW"
#define PATH_BL_FW_AUTO     PATH_ASSET_DIR "/bootloader.fw"
#define PATH_BL_FW_MANUAL   "bootloader.fw"
#define PATH_FANET_FW       PATH_ASSET_DIR "/fanet.xlb"
#define PATH_RELEASE_NOTE   PATH_ASSET_DIR "/release_note.txt"

#define PATH_TOPO_DIR       "agl"
#define PATH_MAP_DIR        "map"
#define PATH_TOPO_INDEX     PATH_SYSTEM_DIR "/agl_index.db"
#define PATH_MAP_INDEX      PATH_SYSTEM_DIR "/map_index.db"
#define PATH_BT_NAMES       PATH_SYSTEM_DIR "/bt_name.db"

#define DEBUG_FILE		"debug.log"
#define UPDATE_FILE 	"STRATO.FW"
#define DEV_MODE_FILE   "DEV_MODE"
#define FORMAT_FILE   	"FORMAT"
#define SKIP_CRC_FILE   "SKIP_CRC"
#define SKIP_STM_FILE   "SKIP_STM"
#define SKIP_ESP_FILE   "SKIP_ESP"
#define KEEP_FW_FILE    "KEEP_FW"

#define ps_malloc(A)	malloc(A)
#define ps_free(A)		free(A)

#define PATH_LEN	128

#define to_radians(degree)      ((degree) / 180.0 * M_PI)
#define to_degrees(radians)     ((radians) * (180.0 / M_PI))

#define min(a,b)                ((a)<(b)?(a):(b))
#define max(a,b)                ((a)>(b)?(a):(b))

#define SWAP_UINT16(x)          ((((x) & 0xFF00) >> 8) | (((x) &0x00FF) << 8))


#define INFO(...)	do{ printf(__VA_ARGS__); printf("\n"); } while(0);
#define WARN(...)	do{ printf("[W]"); printf(__VA_ARGS__); printf("\n"); } while(0);
#define DBG(...)	do{ printf("[D]"); printf(__VA_ARGS__); printf("\n"); } while(0);
#define ERR(...)	do{ printf("[E]"); printf(__VA_ARGS__); printf("\n"); } while(0);

#define ASSERT(cond)	\
	do {	\
		if (!(cond))	\
		{ \
			ERR("Assertion failed %s:%u", __FILE__, __LINE__); \
		} \
	} while(0);

#define FASSERT(A) ASSERT(A)

typedef struct
{
    lv_color_t * buffer;

    int32_t center_lon;
    int32_t center_lat;

    uint16_t zoom;
    bool ready;
    bool not_used;

} map_chunk_t;


typedef enum
{
    FONT_8XL = 0,
    FONT_7XL,
    FONT_6XL,
    FONT_5XL,
    FONT_4XL,
    FONT_3XL,
    FONT_2XL,
    FONT_XL,
    FONT_L,
    FONT_M,
    FONT_S,
    FONT_XS,
    FONT_2XS,

    NUMBER_OF_WIDGET_FONTS,
} gui_font_size_t;

#define FONT_WITH_TEXTS  FONT_XL
#define NUMBER_OF_POI     32

typedef struct
{
    char * name;
    int32_t lat;
    int32_t lon;

    uint32_t uid;

    uint8_t chunk;
    uint8_t type;
    uint8_t magic;
    uint8_t _pad[1];
} map_poi_t;

typedef struct
{
	//styles
	struct
	{
        lv_style_t widget_label;
        lv_style_t widget_unit;
		lv_style_t widget_box;
		lv_style_t list_select;
		lv_style_t note;
		lv_style_t ctx_menu;
		const lv_font_t * widget_fonts[NUMBER_OF_WIDGET_FONTS];
	} styles;

	//map
	struct
	{
        lv_obj_t * canvas;

        map_chunk_t chunks[9];

        map_poi_t poi[NUMBER_OF_POI];
        uint8_t poi_size;

        uint8_t magic;
        uint8_t _pad[1];
	} map;
} gui_t;

extern gui_t gui;

uint32_t HAL_GetTick();
void pix_to_point(lv_point_t point, int32_t map_lon, int32_t map_lat, uint16_t zoom, int32_t * lon, int32_t * lat, lv_obj_t * canvas);
uint32_t get_tmp_filename(char * fname);
bool file_exists(char * file);

#endif /* MAP_COMMON_H_ */
