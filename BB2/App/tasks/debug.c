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

#include "fatfs.h"

void debug_dump(uint8_t * data, uint16_t len)
{
	char tmp[8 * 3 + 3];
	for (uint16_t i = 0; i < len; i++)
	{
		sprintf(tmp + ((i % 8) * 3) + ((i % 8 > 3) ? 1 : 0), "%02X  ", data[i]);
		if (i % 8 == 7 || i + 1 == len)
		{
			tmp[strlen(tmp) - 2] = 0;
			DBG("[%s]", tmp);
		}
	}
}

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

void debug_uart_done()
{
	BaseType_t xHigherPriorityTaskWoken;
	vTaskNotifyGiveFromISR((TaskHandle_t)DebugHandle, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}


uint8_t t_dbg_step = 0xFF;
uint8_t t_dbg_last_res = 0;

void task_Debug(void *argument)
{
	vTaskSuspend(NULL);

	//DMA TX buffer
	static char message[256 + 20];
	FIL debug_file;

	INFO("Started");

	t_dbg_step = 0;

	for(;;)
	{
		debug_msg_t msg;
		t_dbg_step = 1;
		xQueueReceive((QueueHandle_t)queue_DebugHandle, &msg, WAIT_INF);

		char id[] = "DIWE";

		snprintf(message, sizeof(message), "[%c][%s] %s\n", id[msg.type], msg.sender, msg.message);
		free(msg.message);

		t_dbg_step = 2;
		uint8_t res;
		do {
			res = HAL_UART_Transmit_DMA(&debug_uart, (uint8_t *)message, strlen(message));
			t_dbg_last_res = res;
		} while (res != HAL_OK);

		t_dbg_step = 3;

		res = f_open(&debug_file, "debug.log", FA_WRITE | FA_OPEN_APPEND);
		t_dbg_step = 4;
		if (res == FR_OK)
		{
			UINT len;
			f_write(&debug_file, message, strlen(message), &len);
			t_dbg_step = 5;

			f_close(&debug_file);
		}

		t_dbg_step = 6;
		if (t_dbg_step == 0);

//		osDelay(1000);

		ulTaskNotifyTake(true, 10);

	}
}
