/*
 * debug.cc
 *
 *  Created on: Apr 23, 2020
 *      Author: horinek
 */

#include "debug.h"

#include <stdlib.h>
#include <sys/_stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

bool debug_uart_done = true;
bool debug_off = true;

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart == debug_uart)
		debug_uart_done = true;
}

void debug_enable()
{
	debug_off = false;
}

void debug_dump(uint8_t * data, uint16_t len)
{
    char tmp[8 * 3 + 3];
    for (uint16_t i = 0; i < len; i++)
    {
        sprintf(tmp + ((i % 8) * 3) + ((i % 8 > 3) ? 1 : 0), "%02X  ", data[i]);
        if (i % 8 == 7 || i + 1 == len)
        {
            tmp[strlen(tmp) - 2] = 0;
            debug_send(DBG_DEBUG, "[%s]", tmp);
        }
    }
}

void debug_send(uint8_t type, const char *format, ...)
{
	if (debug_off)
		return;

	va_list arp;
	char msg_buff[250];

	va_start(arp, format);
	vsnprintf(msg_buff, sizeof(msg_buff), format, arp);
	va_end(arp);

	uint32_t start = HAL_GetTick();
	while(!debug_uart_done)
	{
		if (HAL_GetTick() - start > 10)
		{
			debug_uart_done = true;
			HAL_UART_DMAStop(debug_uart);

		}
	}

	char id[] = "DIWE";

	static char message[256];
	snprintf(message, sizeof(message), "[%c] %s\n", id[type], msg_buff);

	debug_uart_done = false;
	uint8_t ret = HAL_UART_Transmit_DMA(debug_uart, (uint8_t *)message, strlen(message));
	if (ret != HAL_OK)
	{
		debug_uart_done = true;
		HAL_UART_DMAStop(debug_uart);
	}
}

