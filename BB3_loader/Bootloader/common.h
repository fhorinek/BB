/*
 * gpio_names.h
 *
 *  Created on: Apr 17, 2020
 *      Author: horinek
 */

#ifndef INC_COMMON_H_
#define INC_COMMON_H_

#include "main.h"
#include "structs.h"

#include "stdio.h"
#include "stdlib.h"
#include "stdarg.h"
#include "string.h"
#include "stdbool.h"
#include "math.h"

#include "gpio.h"
#include "dma.h"
#include "usart.h"
#include "tim.h"
#include "mdma.h"
#include "fmc.h"
#include "crc.h"
#include "sdmmc.h"
#include "i2c.h"
#include "octospi.h"
#include "rng.h"

#include "debug.h"
#include "ux_port.h"

#include "redfs.h"
#include "redposix.h"

#include "drivers/sd.h"
#include "drivers/psram.h"

#define	PA0		GPIOA,	GPIO_PIN_0
#define	PA1		GPIOA,	GPIO_PIN_1
#define	PA2		GPIOA,	GPIO_PIN_2
#define	PA3		GPIOA,	GPIO_PIN_3
#define	PA4		GPIOA,	GPIO_PIN_4
#define	PA5		GPIOA,	GPIO_PIN_5
#define	PA6		GPIOA,	GPIO_PIN_6
#define	PA7		GPIOA,	GPIO_PIN_7
#define	PA8		GPIOA,	GPIO_PIN_8
#define	PA9		GPIOA,	GPIO_PIN_9
#define	PA10	GPIOA,	GPIO_PIN_10
#define	PA11	GPIOA,	GPIO_PIN_11
#define	PA12	GPIOA,	GPIO_PIN_12
#define	PA13	GPIOA,	GPIO_PIN_13
#define	PA14	GPIOA,	GPIO_PIN_14
#define	PA15	GPIOA,	GPIO_PIN_15
#define	PB0		GPIOB,	GPIO_PIN_0
#define	PB1		GPIOB,	GPIO_PIN_1
#define	PB2		GPIOB,	GPIO_PIN_2
#define	PB3		GPIOB,	GPIO_PIN_3
#define	PB4		GPIOB,	GPIO_PIN_4
#define	PB5		GPIOB,	GPIO_PIN_5
#define	PB6		GPIOB,	GPIO_PIN_6
#define	PB7		GPIOB,	GPIO_PIN_7
#define	PB8		GPIOB,	GPIO_PIN_8
#define	PB9		GPIOB,	GPIO_PIN_9
#define	PB10	GPIOB,	GPIO_PIN_10
#define	PB11	GPIOB,	GPIO_PIN_11
#define	PB12	GPIOB,	GPIO_PIN_12
#define	PB13	GPIOB,	GPIO_PIN_13
#define	PB14	GPIOB,	GPIO_PIN_14
#define	PB15	GPIOB,	GPIO_PIN_15
#define	PC0		GPIOC,	GPIO_PIN_0
#define	PC1		GPIOC,	GPIO_PIN_1
#define	PC2		GPIOC,	GPIO_PIN_2
#define	PC3		GPIOC,	GPIO_PIN_3
#define	PC4		GPIOC,	GPIO_PIN_4
#define	PC5		GPIOC,	GPIO_PIN_5
#define	PC6		GPIOC,	GPIO_PIN_6
#define	PC7		GPIOC,	GPIO_PIN_7
#define	PC8		GPIOC,	GPIO_PIN_8
#define	PC9		GPIOC,	GPIO_PIN_9
#define	PC10	GPIOC,	GPIO_PIN_10
#define	PC11	GPIOC,	GPIO_PIN_11
#define	PC12	GPIOC,	GPIO_PIN_12
#define	PC13	GPIOC,	GPIO_PIN_13
#define	PC14	GPIOC,	GPIO_PIN_14
#define	PC15	GPIOC,	GPIO_PIN_15
#define	PD0		GPIOD,	GPIO_PIN_0
#define	PD1		GPIOD,	GPIO_PIN_1
#define	PD2		GPIOD,	GPIO_PIN_2
#define	PD3		GPIOD,	GPIO_PIN_3
#define	PD4		GPIOD,	GPIO_PIN_4
#define	PD5		GPIOD,	GPIO_PIN_5
#define	PD6		GPIOD,	GPIO_PIN_6
#define	PD7		GPIOD,	GPIO_PIN_7
#define	PD8		GPIOD,	GPIO_PIN_8
#define	PD9		GPIOD,	GPIO_PIN_9
#define	PD10	GPIOD,	GPIO_PIN_10
#define	PD11	GPIOD,	GPIO_PIN_11
#define	PD12	GPIOD,	GPIO_PIN_12
#define	PD13	GPIOD,	GPIO_PIN_13
#define	PD14	GPIOD,	GPIO_PIN_14
#define	PD15	GPIOD,	GPIO_PIN_15
#define	PE0		GPIOE,	GPIO_PIN_0
#define	PE1		GPIOE,	GPIO_PIN_1
#define	PE2		GPIOE,	GPIO_PIN_2
#define	PE3		GPIOE,	GPIO_PIN_3
#define	PE4		GPIOE,	GPIO_PIN_4
#define	PE5		GPIOE,	GPIO_PIN_5
#define	PE6		GPIOE,	GPIO_PIN_6
#define	PE7		GPIOE,	GPIO_PIN_7
#define	PE8		GPIOE,	GPIO_PIN_8
#define	PE9		GPIOE,	GPIO_PIN_9
#define	PE10	GPIOE,	GPIO_PIN_10
#define	PE11	GPIOE,	GPIO_PIN_11
#define	PE12	GPIOE,	GPIO_PIN_12
#define	PE13	GPIOE,	GPIO_PIN_13
#define	PE14	GPIOE,	GPIO_PIN_14
#define	PE15	GPIOE,	GPIO_PIN_15

#define PIN(A, B)	B
#define PORT(A, B)	A

#define HIGH	GPIO_PIN_SET
#define LOW		GPIO_PIN_RESET

#define INPUT	GPIO_MODE_INPUT
#define OUTPUT	GPIO_MODE_OUTPUT_PP

#define GpioWrite(pin, level)	HAL_GPIO_WritePin(pin, level)
#define GpioRead(pin) 			HAL_GPIO_ReadPin(pin)
void GpioSetDirection(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint16_t direction, uint16_t pull);

bool button_pressed(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
void button_wait(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
void button_confirm(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
bool button_hold(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
bool button_hold_2(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint32_t timeout);

bool file_exists(char * path);
uint64_t file_size(int32_t file);

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
int8_t complement2_8bit(uint8_t in);
int16_t complement2_16bit(uint16_t in);

/**
 * Convert an angle given in degree to radians.
 */
inline double to_radians(double degree)
{
    return degree / 180.0 * M_PI;
}

/**
 * Convert an angle given in radians to degree.
 */
inline double to_degrees(double radians)
{
    return radians * (180.0 / M_PI);
}

uint8_t calc_crc(uint8_t crc, uint8_t key, uint8_t data);

void gui_set_backlight(uint8_t val);
void gui_set_torch(uint8_t val);

void system_reboot();

void clear_dir(char * path);
void remove_dir(char * path);
bool file_is_dir(char * path);

#define PATH_LEN    128

#define UPDATE_FILE 	"strato.fw"
#define DEV_MODE_FILE   "DEV_MODE"
#define FORMAT_FILE   	"FORMAT"
#define SKIP_CRC_FILE   "SKIP_CRC"
#define SKIP_FS_FILE    "SKIP_FS"
#define SKIP_STM_FILE   "SKIP_STM"
#define SKIP_ESP_FILE   "SKIP_ESP"
#define KEEP_FW_FILE    "KEEP_FW"
#define SYSTEM_PATH		"system"
#define ASSET_PATH		SYSTEM_PATH "/assets"

#define MSG_DELAY		2000

#define IN_BOOTLOADER

#define NO_OPTI __attribute__((optimize("O0")))

extern bool development_mode;

void bat_check_step();

void app_sleep();
void app_main_entry(ULONG id);

#define POWER_ON_USB            0
#define POWER_ON_BUTTON         1
#define POWER_ON_TORCH          2
#define POWER_ON_BOOST          3
#define POWER_ON_REBOOT         4

extern uint8_t power_on_mode;

#endif /* INC_COMMON_H_ */
