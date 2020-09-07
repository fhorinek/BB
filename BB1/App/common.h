/*
 * gpio_names.h
 *
 *  Created on: Apr 17, 2020
 *      Author: horinek
 */

#ifndef INC_COMMON_H_
#define INC_COMMON_H_

#include "main.h"

#include "stdio.h"
#include "stdlib.h"
#include "stdarg.h"
#include "stdbool.h"

#include "string.h"

#include "math.h"

#include "cmsis_os.h"

#include "common_gpio.h"

#include "fatfs.h"

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

struct vector_i16_t
{
    int16_t x;
    int16_t y;
    int16_t z;
};

struct vector_i32_t
{
    int32_t x;
    int32_t y;
    int32_t z;
};

struct vector_float_t
{
    float x;
    float y;
    float z;
};


//RTOS Tasks
extern osThreadId_t GUIHandle;
extern osThreadId_t DebugHandle;
extern osThreadId_t GNSSHandle;
extern osThreadId_t TestHandle;
extern osThreadId_t UsbHandle;

//RTOS Queue
extern osMessageQueueId_t queue_DebugHandle;

//RTOS defs
#define WAIT_INF	portMAX_DELAY

//Paths
#define PATH_CONFIG_DIR		"config"
#define PATH_DEVICE_CFG		PATH_CONFIG_DIR "/device.cfg"
#define PATH_PAGES_DIR		PATH_CONFIG_DIR "/pages"


//RGB and backlight timer
extern TIM_HandleTypeDef htim2;

#define tim_rgb_and_bck		htim2
#define tim_bckl_channel 	TIM_CHANNEL_1
#define tim_red_channel 	TIM_CHANNEL_2
#define tim_green_channel 	TIM_CHANNEL_3
#define tim_blue_channel 	TIM_CHANNEL_4

//debug UART
#define debug_uart		huart1
extern UART_HandleTypeDef huart1;

//GPS Uart
#define gnss_uart		huart4
extern UART_HandleTypeDef huart4;

//FANET Uart
#define fanet_uart		huart8
extern UART_HandleTypeDef huart8;

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

int8_t complement7(uint8_t in);

char * find_in_file(FIL * f, char * key, char * def, char * buff, uint16_t len);

/**
 * Convert an angle given in degree to radians.
 */
#define to_radians(degree) (degree / 180.0 * M_PI)

/**
 * Convert an angle given in radians to degree.
 */
#define to_degrees(radians) (radians * (180.0 / M_PI))

#include "debug.h"

#endif /* INC_COMMON_H_ */
