/*
 * safe_uart.h
 *
 *  Created on: 21. 2. 2022
 *      Author: horinek
 */

#ifndef DRIVERS_SAFE_UART_H_
#define DRIVERS_SAFE_UART_H_

#include "common.h"
#include "rb.h"

typedef struct
{
    UART_HandleTypeDef * uart;
    rb_handle_t rb;
    osSemaphoreId_t lock;
    uint8_t * dma_buf;
    uint32_t dma_size;
} safe_uart_t;

void su_init(safe_uart_t * su, UART_HandleTypeDef * uart, uint32_t buffer_len, uint32_t dma_len);
void su_clear(safe_uart_t * su);

bool su_write(safe_uart_t * su, uint8_t * data, uint32_t len);
void su_tx_done(safe_uart_t * su);

#endif /* DRIVERS_SAFE_UART_H_ */
