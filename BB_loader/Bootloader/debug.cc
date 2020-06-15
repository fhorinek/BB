/*
 * debug.cc
 *
 *  Created on: Apr 23, 2020
 *      Author: horinek
 */

#include "debug.h"

#include <stdlib.h>
#include <sys/_stdint.h>
#include <cstdarg>
#include <cstdio>
#include <cstring>

bool debug_uart_done = true;

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart == &debug_uart)
		debug_uart_done = true;
}


void debug_send(uint8_t type, const char *format, ...)
{
	va_list arp;
	char msg_buff[250];
	uint16_t length;

	va_start(arp, format);
	length = vsnprintf(msg_buff, sizeof(msg_buff), format, arp);
	va_end(arp);

	while(!debug_uart_done);

	char id[] = "DIWE";

	static char message[256];
	sprintf(message, "[%c] %s\n", id[type], msg_buff);

	debug_uart_done = false;
	HAL_UART_Transmit_DMA(&debug_uart, (uint8_t *)message, strlen(message));
}

