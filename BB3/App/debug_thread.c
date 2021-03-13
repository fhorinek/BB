/*
 * gui.cc
 *
 *  Created on: Apr 23, 2020
 *      Author: horinek
 */

#include <debug_thread.h>
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
			debug_send(DBG_DEBUG, "[%s]", tmp);
		}
	}
}

#define DEBUG_TX_BUFFER    (1024 * 4)
#define DEBUG_RX_BUFFER     32
static char debug_tx_buffer[DEBUG_TX_BUFFER];
static uint16_t debug_tx_buffer_write_index = 0;
static uint16_t debug_tx_buffer_read_index = 0;
static uint16_t debug_tx_buffer_lenght = 0;

static char debug_rx_buffer[DEBUG_RX_BUFFER];
static uint16_t debug_rx_read_index = 0;

osSemaphoreId_t debug_dma_done;
osSemaphoreId_t debug_new_message;

bool debug_thread_ready = false;

void debug_fault(const char *format, ...)
{
    //wait for uart to be ready, or force it
    uint32_t loops = 0;
    while (HAL_UART_GetState(debug_uart) != HAL_UART_STATE_READY)
    {
        if (loops++ > 10000)
        {
            HAL_UART_Abort(debug_uart);
        }
    }

    va_list arp;

    char * name;

    if (debug_thread_ready)
    {
        if (xPortIsInsideInterrupt())
            name = "IRQ";
        else
            name = pcTaskGetName(NULL);
    }
    else
    {
        name = "BOOT";
    }

    uint16_t head_lenght = snprintf(debug_tx_buffer, sizeof(debug_tx_buffer), "\n[%s][FAULT] ", name);

    //Message boddy
    va_start(arp, format);
    uint16_t body_lenght = vsnprintf(debug_tx_buffer + head_lenght, sizeof(debug_tx_buffer) - head_lenght, format, arp);
    va_end(arp);

    uint16_t total_lenght = head_lenght + body_lenght;

    HAL_UART_Transmit(debug_uart, (uint8_t *)debug_tx_buffer, total_lenght, 100);
    osSemaphoreRelease(debug_dma_done);
}

ALIGN static char debug_label[] = "DIWE";

void debug_send(uint8_t type, const char *format, ...)
{
	va_list arp;

    char msg[1024];

    char * name;

    if (debug_thread_ready)
    {
        if (xPortIsInsideInterrupt())
            name = "IRQ";
        else
            name = pcTaskGetName(NULL);
    }
    else
    {
        name = "BOOT";
    }

    uint16_t head_lenght = snprintf(msg, sizeof(msg), "\n[%s][%c] ", name, debug_label[type]);

    //Message boddy
    va_start(arp, format);
    uint16_t body_lenght = vsnprintf(msg + head_lenght, sizeof(msg) - head_lenght, format, arp);
    va_end(arp);

    uint16_t total_lenght = head_lenght + body_lenght;

    if (debug_thread_ready)
    {
        if (!xPortIsInsideInterrupt())
        {
            portENTER_CRITICAL();
        }

        if (DEBUG_TX_BUFFER - debug_tx_buffer_lenght < total_lenght)
            total_lenght = DEBUG_TX_BUFFER - debug_tx_buffer_lenght;

        if (total_lenght > 0)
        {
            uint16_t space_avalible = DEBUG_TX_BUFFER - debug_tx_buffer_write_index;
            if (space_avalible < total_lenght)
            {
                memcpy(debug_tx_buffer + debug_tx_buffer_write_index, msg, space_avalible);
                memcpy(debug_tx_buffer, msg + space_avalible, total_lenght - space_avalible);
                debug_tx_buffer_write_index = total_lenght - space_avalible;
            }
            else
            {
                memcpy(debug_tx_buffer + debug_tx_buffer_write_index, msg, total_lenght);
                debug_tx_buffer_write_index += total_lenght;
            }

            debug_tx_buffer_lenght += total_lenght;

            osSemaphoreRelease(debug_new_message);
        }
        else
        {
            FAULT("Debug buffer full!");
        }

        if (!xPortIsInsideInterrupt())
        {
            portEXIT_CRITICAL();
        }
    }
    else
    {
        HAL_UART_Transmit(debug_uart, (uint8_t *)msg, total_lenght, 100);
    }
}

void debug_uart_done()
{
    osSemaphoreRelease(debug_dma_done);

    if (debug_tx_buffer_lenght > 0)
        osSemaphoreRelease(debug_new_message);
}

uint16_t debug_get_waiting()
{
    uint16_t dma_index = DEBUG_RX_BUFFER - __HAL_DMA_GET_COUNTER(debug_uart->hdmarx);

    //Get number of bytes waiting in buffer
    if (debug_rx_read_index > dma_index)
    {
        return DEBUG_RX_BUFFER - debug_rx_read_index + dma_index;
    }
    else
    {
        return dma_index - debug_rx_read_index;
    }
}

uint8_t debug_read_byte()
{
    uint8_t byte = debug_rx_buffer[debug_rx_read_index];
    debug_rx_read_index = (debug_rx_read_index + 1) % DEBUG_RX_BUFFER;
    return byte;
}


void thread_debug_start(void *argument)
{
    debug_dma_done = osSemaphoreNew(1, 0, NULL);
    debug_new_message = osSemaphoreNew(1, 0, NULL);
    vQueueAddToRegistry(debug_dma_done, "debug_dma_done");
    vQueueAddToRegistry(debug_new_message, "debug_new_message");

    HAL_UART_Receive_DMA(debug_uart, (uint8_t *)debug_rx_buffer, DEBUG_RX_BUFFER);

    osSemaphoreRelease(debug_dma_done);

//	FIL debug_file;

    debug_thread_ready = true;
	INFO("\n\n\n-----------------------------------------------------------------------------");

	for(;;)
	{
	    osSemaphoreAcquire(debug_new_message, WAIT_INF);

        uint16_t to_transmit = debug_tx_buffer_lenght;

        if (to_transmit > 0)
        {
            if (to_transmit > DEBUG_TX_BUFFER - debug_tx_buffer_read_index)
                to_transmit = DEBUG_TX_BUFFER - debug_tx_buffer_read_index;

            portENTER_CRITICAL();
            HAL_UART_Transmit_DMA(debug_uart, (uint8_t *)(debug_tx_buffer + debug_tx_buffer_read_index), to_transmit);
            debug_tx_buffer_lenght -= to_transmit;
            debug_tx_buffer_read_index = (debug_tx_buffer_read_index + to_transmit) % DEBUG_TX_BUFFER;
            portEXIT_CRITICAL();

            osSemaphoreAcquire(debug_dma_done, WAIT_INF);
	    }
	}
}
