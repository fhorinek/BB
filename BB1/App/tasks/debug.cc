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


extern "C" void task_Debug(void *argument);
extern "C" void debug_tx_irq_done();


void debug_send(uint8_t type, const char *format, ...)
{
	debug_msg_t msg;

	msg.sender = pcTaskGetName(NULL);
	msg.type = type;

	va_list arp;
	char msg_buff[256];
	int16_t length;

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

	msg.messaage = (char *) malloc(length + 1);
	strcpy(msg.messaage, msg_buff);

	if (xPortIsInsideInterrupt())
	{
		xQueueSendFromISR((QueueHandle_t)queue_DebugHandle, &msg, NULL);
	}
	else
	{
		xQueueSend((QueueHandle_t)queue_DebugHandle, &msg, WAIT_INF);
	}
}

void debug_tx_irq_done()
{
	vTaskNotifyGiveFromISR((TaskHandle_t)DebugHandle, NULL);
}

void task_Debug(void *argument)
{
	//DMA TX buffer
	static char message[256 + 20];

	INFO("Started");

	for(;;)
	{
		debug_msg_t msg;
		xQueueReceive((QueueHandle_t)queue_DebugHandle, &msg, WAIT_INF);

		char id[] = "DIWE";

		sprintf(message, "[%c][%s] %s\n", id[msg.type], msg.sender, msg.messaage);
		free(msg.messaage);

		//HAL_UART_Transmit(&huart1, (uint8_t *)message, strlen(message), 100);
		if (HAL_UART_Transmit_DMA(&huart1, (uint8_t *)message, strlen(message)) != HAL_OK)
		{
			Error_Handler();
		}
		ulTaskNotifyTake(false, 10);
	}
}
