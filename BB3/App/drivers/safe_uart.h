/*
 * safe_uart.h
 *
 *  Created on: 21. 2. 2022
 *      Author: horinek
 */

#ifndef DRIVERS_SAFE_UART_H_
#define DRIVERS_SAFE_UART_H_

#include "common.h"

typedef struct
{
    QueueHandle_t queue;
    uint8_t * last_data;
    UART_HandleTypeDef * uart;
} safe_uart_t;

typedef struct
{
    uint8_t * data;
    uint32_t len;
} safe_uart_item_t;

void su_init(safe_uart_t * su, UART_HandleTypeDef * uart, uint16_t queue_len);
void su_clear(safe_uart_t * su);

void su_write(safe_uart_t * su, uint8_t * data, uint32_t len, bool urgent);
void su_tx_done(safe_uart_t * su);

#endif /* DRIVERS_SAFE_UART_H_ */
