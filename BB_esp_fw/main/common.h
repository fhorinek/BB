/*
 * common.h
 *
 *  Created on: 3. 12. 2020
 *      Author: horinek
 */

#ifndef MAIN_COMMON_H_
#define MAIN_COMMON_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>


#include "../main/debug.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_http_server.h"

#include "../../BB3/App/drivers/esp/protocol_def.h"

#define HIGH    1
#define LOW     0

//i2c
#define I2C_PORT            I2C_NUM_0
#define I2C_SCL             GPIO_NUM_32
#define I2C_SDA             GPIO_NUM_33
#define I2C_MASTER_FREQ_HZ  100000

//i2s
#define I2S_PORT            I2S_NUM_0
#define I2S_BCLK            GPIO_NUM_27 //BCLK
#define I2S_WS              GPIO_NUM_21 //LRCLK
#define I2S_DO              GPIO_NUM_4  //SDIN

//spi
#define SPI_PORT            SPI2_HOST
#define SPI_DMA_CHAN        2
#define SPI_MOSI            GPIO_NUM_23
#define SPI_MISO            GPIO_NUM_19
#define SPI_SCLK            GPIO_NUM_18
#define SPI_CS              GPIO_NUM_5

//gpio
#define BOOST_EN            GPIO_NUM_25 //ACTIVE HIGH
#define AMP_SD              GPIO_NUM_22 //ACTIVE HIGH
#define FANET_BOOT0         GPIO_NUM_26 //ACTIVE HIGH


//#define UART_BAUDRATE       921600
#define UART_BAUDRATE       115200

#define WAIT_INF    portMAX_DELAY

#define DEVICE_HOSTNAME	"strato"
#define DEVICE_INSTANCE	"SkyBean Strato"

uint8_t calc_crc(uint8_t crc, uint8_t key, uint8_t data);
void fanet_boot0_ctrl(proto_fanet_boot0_ctrl_t * packet);

void * ps_malloc(uint32_t size);

bool read_post(char * data, char * key, char * value, uint16_t value_len);
bool read_post_int32(char * data, char * key, int32_t * value);
bool read_post_int16(char * data, char * key, int16_t * value);

#define min(a,b) 	((a)<(b)?(a):(b))
#define max(a,b) 	((a)>(b)?(a):(b))
#define abs(x) 		((x)>0?(x):-(x))

typedef struct
{
	uint8_t amp_ok;
	uint8_t server_ok;
} esp_system_status_t;

extern esp_system_status_t system_status;

void print_free_memory(char * label);
uint32_t get_ms();
void task_sleep(uint32_t ms);

#endif /* MAIN_COMMON_H_ */
