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


#define HIGH    1
#define LOW     0

#define UART_BAUDRATE	115200

#define WAIT_INF    portMAX_DELAY

uint8_t calc_crc(uint8_t crc, uint8_t key, uint8_t data);

#endif /* MAIN_COMMON_H_ */
