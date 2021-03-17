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

//os
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"
#include "fatfs.h"

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

#define SWAP_UINT24(x) ((((x) & 0xFF0000) >> 16) | ((x) & 0x00FF00) | (((x) & 0x0000FF) << 16))
#define SWAP_UINT16(x) ((((x) & 0xFF00) >> 8) | (((x) &0x00FF) << 8))

extern bool system_power_off;

//RTOS Macros
#define define_thread(NAME, FUNC, STACK, PRIO)  \
osThreadId_t FUNC = NULL;                 \
static uint32_t FUNC ## _stack[STACK];  \
static StaticTask_t FUNC ## _cb; \
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

#define ALIGN     alignas(4)

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

extern osThreadId_t SystemHandle;
#define thread_system   (osThreadId_t)SystemHandle

//RTOS Queue
extern osMessageQueueId_t queue_DebugHandle;

//RTOS semaphores
extern osSemaphoreId_t lock_fc_global;

//RTOS defs
#define WAIT_INF	portMAX_DELAY
//do not use in ISR
//#define FC_ATOMIC_ACCESS		for(osSemaphoreAcquire(fc_global_lockHandle, 0); osSemaphoreGetCount(fc_global_lockHandle) == 0; osSemaphoreRelease(fc_global_lockHandle))
#define FC_ATOMIC_ACCESS
//Paths
#define PATH_CONFIG_DIR		"config"
#define PATH_DEVICE_CFG		PATH_CONFIG_DIR "/device.cfg"
#define PATH_PAGES_DIR      PATH_CONFIG_DIR "/pages"
#define PATH_SCREENSHOT     "scrshot"


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

char * find_in_file(FIL * f, char * key, char * def, char * buff, uint16_t len);
#define to_radians(degree) (degree / 180.0 * M_PI)
#define to_degrees(radians) (radians * (180.0 / M_PI))

uint8_t calc_crc(uint8_t crc, uint8_t key, uint8_t data);
uint32_t calc_crc32(uint32_t * data, uint32_t size);

void rtos_timer_elapsed();

#define DEVICE_ID   ((HAL_GetREVID() << 24) | HAL_GetDEVID())

#include <debug_thread.h>

#endif /* INC_COMMON_H_ */
