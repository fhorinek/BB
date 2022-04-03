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
    char s[18] = {0};

    char tmp[16 * 3 + 3];
    for (uint16_t i = 0; i < len; i++)
    {
        sprintf(tmp + ((i % 16) * 3) + ((i % 16 > 7) ? 1 : 0), "%02X  ", data[i]);
        if (i % 16 == 0) memset(s, ' ', 17);

        s[(i % 16) + ((i % 16 > 7) ? 1 : 0)] = (data[i] > 32 && data[i] <= 127) ? data[i] : '.';


        if (i % 16 == 15 || i + 1 == len)
        {
            tmp[strlen(tmp) - 2] = 0;
            debug_send(DBG_DEBUG, "[%-48s] %s", tmp, s);
        }
    }
}

void debug_send(uint8_t type, const char *format, ...)
{
	if (debug_off)
		return;

	va_list arp;
	static char msg_buff[1024];

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

	static char message[1024];
	snprintf(message, sizeof(message), "[%lu][%c] %s\n", HAL_GetTick(), id[type], msg_buff);

	debug_uart_done = false;
	uint8_t ret = HAL_UART_Transmit_DMA(debug_uart, (uint8_t *)message, strlen(message));
	if (ret != HAL_OK)
	{
		debug_uart_done = true;
		HAL_UART_DMAStop(debug_uart);
	}
}

