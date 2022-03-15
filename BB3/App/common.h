/*
 * gpio_names.h
 *
 *  Created on: Apr 17, 2020
 *      Author: horinek
 */

#ifndef INC_COMMON_H_
#define INC_COMMON_H_

#include "main.h"

//std libs
#include "stdio.h"
#include "stdlib.h"
#include "stdarg.h"
#include "stdbool.h"
#include "string.h"
#include "math.h"
#include "stdalign.h"

//ll drivers
#include "dma.h"
#include "usart.h"
#include "tim.h"
#include "octospi.h"
#include "i2c.h"
#include "rtc.h"
#include "spi.h"
#include "crc.h"
#include "sdmmc.h"

//os
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"
#include "fatfs.h"
#include "queue.h"


#include "drivers/psram.h"
#include "config/config.h"
#include "config/db.h"
#include "common_gpio.h"


typedef union
{
    uint64_t uint64;
    uint32_t uint32[2];
    uint8_t uint8[4];
} byte8;

typedef union
{
    uint32_t uint32;
    int32_t int32;
    uint8_t uint8[4];
} byte4;

typedef union
{
    uint16_t uint16;
    int16_t int16;
    uint8_t uint8[2];
} byte2;

typedef struct
{
    int16_t x;
    int16_t y;
    int16_t z;
    uint8_t _pad[2];
} vector_i16_t;

typedef struct
{
    int32_t x;
    int32_t y;
    int32_t z;
} vector_i32_t;

typedef struct
{
    float X;
    float Y;
} vector_2d_float_t;

typedef struct
{
    float x;
    float y;
    float z;
} vector_float_t;

typedef struct
{
    float q0;
    float q1;
    float q2;
    float q3;
} quaternion_t;

#define BLINK(A)                (HAL_GetTick() % A > (A / 2))
#define min(a,b)                ((a)<(b)?(a):(b))
#define max(a,b)                ((a)>(b)?(a):(b))
#define abs(x)                  ((x)>0?(x):-(x))
#define to_radians(degree)      ((degree) / 180.0 * M_PI)
#define to_degrees(radians)     ((radians) * (180.0 / M_PI))
#define ISDIGIT(c)              ((c) - '0' + 0U <= 9U)
#define ISALPHA(c)              (((c) | 32) - 'a' + 0U <= 'z' - 'a' + 0U)
#define CLAMP(val, min, max)    ((val < min) ? (min) : ((val > max) ? max : val))
#define SWAP_UINT24(x)          ((((x) & 0xFF0000) >> 16) | ((x) & 0x00FF00) | (((x) & 0x0000FF) << 16))
#define SWAP_UINT16(x)          ((((x) & 0xFF00) >> 8) | (((x) &0x00FF) << 8))

#define __align __attribute__ ((aligned (4)))
#define NO_OPTI	__attribute__((optimize("O0")))

extern bool system_power_off;

//RTOS Macros
#define define_thread(NAME, FUNC, STACK, PRIO)  \
osThreadId_t FUNC = NULL;                 \
static uint32_t FUNC ## _stack[STACK];  \
StaticTask_t FUNC ## _cb; \
const osThreadAttr_t FUNC ## _attr = {  \
  .name = NAME,                         \
  .stack_mem = FUNC ## _stack,          \
  .stack_size = sizeof(FUNC ## _stack), \
  .cb_mem = &FUNC ## _cb,               \
  .cb_size = sizeof(FUNC ## _cb),       \
  .priority = (osPriority_t) PRIO,      \
};

#define start_thread(FUNC)  \
    FUNC = osThreadNew(FUNC ## _start, NULL, &FUNC ## _attr);

//RTOS Threads
extern osThreadId_t thread_debug;
extern osThreadId_t thread_mems;
extern osThreadId_t thread_gnss;
extern osThreadId_t thread_gui;
extern osThreadId_t thread_map;
extern osThreadId_t thread_esp;
extern osThreadId_t thread_esp_spi;
extern osThreadId_t thread_usb;

extern const osThreadAttr_t thread_esp_spi_attr;
extern const osThreadAttr_t thread_map_attr;
extern const osThreadAttr_t thread_esp_attr;
extern const osThreadAttr_t thread_gnss_attr;
extern const osThreadAttr_t thread_mems_attr;

extern osThreadId_t SystemHandle;
#define thread_system   (osThreadId_t)SystemHandle

//RTOS semaphores

//RTOS defs
#define WAIT_INF	portMAX_DELAY
//do not use in ISR
//FC_ATOMIC_ACCESS is for loop, you can't use break in there

#define FC_ATOMIC_ACCESS		for(uint8_t __first = fc_lock_acquire(); __first == true; __first = fc_lock_release())
//#define FC_ATOMIC_ACCESS
//Paths
#define	PATH_LEN	128

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
#define PATH_MAP_CACHE_DIR  PATH_CACHE_DIR "/map"

#define PATH_CRASH_DIR      "crash"
#define PATH_CRASH_DUMP     PATH_CRASH_DIR "/dump.bin"
#define PATH_CRASH_INFO     PATH_CRASH_DIR "/info.txt"
#define PATH_CRASH_FILES    PATH_CRASH_DIR "/files.txt"
#define PATH_CRASH_LOG      PATH_CRASH_DIR "/debug.log"

#define PATH_ASSET_DIR      PATH_SYSTEM_DIR "/assets"
#define PATH_DEFAULTS_DIR   PATH_ASSET_DIR "/defaults"
#define PATH_LOGS_DIR       "logs"
#define PATH_NEW_FW         PATH_ASSET_DIR "/NEW"
#define PATH_BL_FW_AUTO     PATH_ASSET_DIR "/bootloader.fw"
#define PATH_BL_FW_MANUAL   "bootloader.fw"
#define PATH_FANET_FW       PATH_ASSET_DIR "/fanet.xlb"
#define PATH_RELEASE_NOTE   PATH_ASSET_DIR "/release_note.txt"
#define PATH_TTS_DIR        PATH_ASSET_DIR "/tts/en"

#define PATH_TOPO_DIR       "agl"
#define PATH_MAP_DIR        "map"
#define PATH_TOPO_INDEX     PATH_SYSTEM_DIR "/agl_index.db"
#define PATH_MAP_INDEX      PATH_SYSTEM_DIR "/map_index.db"
#define PATH_BT_NAMES       PATH_SYSTEM_DIR "/bt_name.db"

#define IMU_LOG         "imu.csv"
#define DEBUG_FILE		"debug.log"
#define UPDATE_FILE 	"STRATO.FW"
#define DEV_MODE_FILE   "DEV_MODE"
#define FORMAT_FILE   	"FORMAT"
#define SKIP_CRC_FILE   "SKIP_CRC"
#define SKIP_STM_FILE   "SKIP_STM"
#define SKIP_ESP_FILE   "SKIP_ESP"
#define KEEP_FW_FILE    "KEEP_FW"

#define DEVEL_ACTIVE    (file_exists(DEV_MODE_FILE))

//simple functions
uint8_t hex_to_num(uint8_t c);
bool start_with(char * s1, const char * s2);
char * find_comma(char * str);

uint16_t atoi_c(char * str);
float atoi_f(char * str);
uint32_t atoi_n(char * str, uint8_t n);
uint8_t atoi_hex8(char * buffer);
uint32_t atoi_hex32(char * buffer);


int8_t complement2_7bit(uint8_t in);
int16_t complement2_16bit(uint16_t in);

bool file_exists(char * path);
bool file_isdir(char * path);
void touch(char * path);
char * find_in_file_sep(FIL * f, char * key, char * def, char * buff, uint16_t len, char separator);
char * find_in_file(FIL * f, char * key, char * def, char * buff, uint16_t len);

uint8_t calc_crc(uint8_t crc, uint8_t key, uint8_t data);
uint32_t calc_crc32(uint32_t * data, uint32_t size);

void rtos_timer_elapsed();

#define TEMP_NAME_LEN       21
void get_tmp_path(char * fname, uint32_t id);
uint32_t get_tmp_filename(char * fname);

void clear_dir(char * path);
void remove_dir(char * path);
bool copy_file(char * src, char * dst);
void copy_dir(char * src, char * dst);
void copy_dir_when_absent(char * src, char * dst);

bool read_value(char * data, char * key, char * value, uint16_t value_len);

float table_sin(uint16_t angle);
float table_cos(uint16_t angle);

void system_free(void * ptr);
void system_poweroff();
void system_reboot();
void system_reboot_bl();
void system_wait_for_handle(osThreadId_t * handle);

uint8_t nmea_checksum(char *s);

void str_join(char * dst, uint8_t cnt, ...);


#define simple_memcpy(dst, src, len) \
do { \
    for (size_t __simple_memcpy_i = 0; __simple_memcpy_i < len; __simple_memcpy_i++) \
        ((uint8_t *)dst)[__simple_memcpy_i] = ((uint8_t *)src)[__simple_memcpy_i]; \
} while(0);

#define safe_memcpy(dst, src, len) \
do { \
    if ((uint32_t)dst % 4 == 0 && (uint32_t)src % 4 == 0) \
    { \
        memcpy(dst, src, len); \
    } \
    else \
    { \
        ASSERT(0); \
        if ((uint32_t)dst % 4 != 0) \
            ERR("safe_memcpy dst is %08X", dst); \
        if ((uint32_t)src % 4 != 0) \
            ERR("safe_memcpy src is %08X", src); \
        \
        simple_memcpy(dst, src, len);\
    } \
} while(0);


#include "system/debug_thread.h"
#include "system/bsod.h"

#endif /* INC_COMMON_H_ */
