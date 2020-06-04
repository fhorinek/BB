/*
 * gui.cc
 *
 *  Created on: Apr 23, 2020
 *      Author: horinek
 */

#include "../debug.h"

#include <portmacro.h>
#include <queue.h>
#include <stdlib.h>
#include <sys/_stdint.h>
#include <task.h>
#include <cstdarg>
#include <cstdio>
#include <cstring>

#include "fatfs.h"

extern "C" void task_Debug(void *argument);

void debug_send(uint8_t type, const char *format, ...)
{
	debug_msg_t msg;

	msg.sender = pcTaskGetName(NULL);
	msg.type = type;

	va_list arp;
	char msg_buff[256];
	uint16_t length;

	va_start(arp, format);
	length = vsnprintf(msg_buff, sizeof(msg_buff), format, arp);
	va_end(arp);

	if (length >= sizeof(msg_buff))
	{
		//message too long
		length = sizeof(msg_buff);
		msg_buff[length - 1] = 0;

		debug_send(DBG_ERROR, "Next message is too long!");
	}

	msg.message = (char *) malloc(length + 1);
	strcpy(msg.message, msg_buff);

	if (xPortIsInsideInterrupt())
	{
		xQueueSendFromISR((QueueHandle_t)queue_DebugHandle, &msg, NULL);
	}
	else
	{
		xQueueSend((QueueHandle_t)queue_DebugHandle, &msg, WAIT_INF);
	}
}


void task_Debug(void *argument)
{
	//DMA TX buffer
	static char message[256 + 20];
	FIL debug_file;

	INFO("Started");

	for(;;)
	{
		debug_msg_t msg;
		xQueueReceive((QueueHandle_t)queue_DebugHandle, &msg, WAIT_INF);

		char id[] = "DIWE";

		sprintf(message, "[%c][%s] %s\n", id[msg.type], msg.sender, msg.message);
		free(msg.message);

		uint8_t res;
		do {
			res = HAL_UART_Transmit(&debug_uart, (uint8_t *)message, strlen(message), 100);
		} while (res != HAL_OK);

		res = f_open(&debug_file, "debug.log", FA_WRITE | FA_OPEN_APPEND);
		if (res == FR_OK)
		{
			UINT len;
			f_write(&debug_file, message, strlen(message), &len);
			f_close(&debug_file);
		}
	}
}
