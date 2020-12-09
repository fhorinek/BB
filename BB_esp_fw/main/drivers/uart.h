/*
 * uart.h
 *
 *  Created on: 3. 12. 2020
 *      Author: horinek
 */

#ifndef MAIN_DRIVERS_UART_H_
#define MAIN_DRIVERS_UART_H_

#include "../common.h"

void uart_init();
void uart_send(uint8_t *data, uint16_t len);

#endif /* MAIN_DRIVERS_UART_H_ */
