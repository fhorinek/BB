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
} vector_i16_t;

typedef struct
{
    int32_t x;
    int32_t y;
    int32_t z;
} vector_i32_t;

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

typedef struct
{
  uint32_t r0;
  uint32_t r1;
  uint32_t r2;
  uint32_t r3;
  uint32_t r12;
  uint32_t lr;
  uint32_t sp;
  uint32_t xpsr;
} context_frame_t;

#define SWAP_UINT24(x) ((((x) & 0xFF0000) >> 16) | ((x) & 0x00FF00) | (((x) & 0x0000FF) << 16))
#define SWAP_UINT16(x) ((((x) & 0xFF00) >> 8) | (((x) &0x00FF) << 8))

#define __align __attribute__ ((aligned (4)))

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

#define BLINK(A)	(HAL_GetTick() % A > (A / 2))

//RTOS Threads
extern osThreadId_t thread_debug;
extern osThreadId_t thread_mems;
extern osThreadId_t thread_gnss;
extern osThreadId_t thread_gui;
extern osThreadId_t thread_map;
extern osThreadId_t thread_esp;
extern osThreadId_t thread_esp_spi;
extern osThreadId_t thread_usb;

//extern StaticTask_t thread_gui_cb;

extern const osThreadAttr_t thread_esp_spi_attr;
extern const osThreadAttr_t thread_map_attr;

extern osThreadId_t SystemHandle;
#define thread_system   (osThreadId_t)SystemHandle

//RTOS Queue

//RTOS semaphores

//RTOS defs
#define WAIT_INF	portMAX_DELAY
//do not use in ISR
#define FC_ATOMIC_ACCESS		for(uint8_t __s = osSemaphoreAcquire(fc.lock, WAIT_INF); __s == osOK; __s = !osSemaphoreRelease(fc.lock))
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

#define PATH_ASSET_DIR      PATH_SYSTEM_DIR "/assets"
#define PATH_LOGS_DIR       "logs"
#define PATH_NEW_FW         PATH_ASSET_DIR "/NEW"
#define PATH_RELEASE_NOTE   PATH_ASSET_DIR "/release_note.txt"

#define PATH_TOPO_DIR       "agl"
#define PATH_MAP_DIR        "map"
#define PATH_TOPO_INDEX     PATH_SYSTEM_DIR "/agl_index.db"
#define PATH_MAP_INDEX      PATH_SYSTEM_DIR "/map_index.db"

#define DEBUG_FILE		"debug.log"
#define UPDATE_FILE 	"STRATO.FW"
#define DEV_MODE_FILE   "DEV_MODE"
#define FORMAT_FILE   	"FORMAT"
#define SKIP_CRC_FILE   "SKIP_CRC"
#define SKIP_STM_FILE   "SKIP_STM"
#define SKIP_ESP_FILE   "SKIP_ESP"
#define KEEP_FW_FILE    "KEEP_FW"


#define TEMP_NAME_LEN       21

//simple functions
uint8_t hex_to_num(uint8_t c);
bool start_with(char * s1, const char * s2);
char * find_comma(char * str);

#define ISDIGIT(c) ((c) - '0' + 0U <= 9U)
uint16_t atoi_c(char * str);
float atoi_f(char * str);
uint32_t atoi_n(char * str, uint8_t n);
uint8_t atoi_hex8(char * buffer);
uint32_t atoi_hex32(char * buffer);

#define min(a,b) 	((a)<(b)?(a):(b))
#define max(a,b) 	((a)>(b)?(a):(b))
#define abs(x) 		((x)>0?(x):-(x))

int8_t complement2_7bit(uint8_t in);
int16_t complement2_16bit(uint16_t in);

bool file_exists(char * path);
bool file_isdir(char * path);
bool touch(char * path);

char * find_in_file_sep(FIL * f, char * key, char * def, char * buff, uint16_t len, char separator);
char * find_in_file(FIL * f, char * key, char * def, char * buff, uint16_t len);
#define to_radians(degree) (degree / 180.0 * M_PI)
#define to_degrees(radians) (radians * (180.0 / M_PI))

uint8_t calc_crc(uint8_t crc, uint8_t key, uint8_t data);
uint32_t calc_crc32(uint32_t * data, uint32_t size);

void rtos_timer_elapsed();

void get_tmp_path(char * fname, uint32_t id);
uint32_t get_tmp_filename(char * fname);

void clear_dir(char * path);
bool copy_file(char * src, char * dst);

bool read_value(char * data, char * key, char * value, uint16_t value_len);

float table_sin(uint16_t angle);
float table_cos(uint16_t angle);

void system_reboot();
void system_reboot_bl();

#include <system/debug_thread.h>

#endif /* INC_COMMON_H_ */
