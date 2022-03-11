/*
 * safe_uart.c
 *
 *  Created on: 21. 2. 2022
 *      Author: horinek
 */

#define DEBUG_LEVEL     DBG_DEBUG

#include "safe_uart.h"
#include "FreeRTOS.h"


void su_init(safe_uart_t * su, UART_HandleTypeDef * uart, uint32_t buffer_len, uint32_t dma_len)
{
    su->uart = uart;
    rb_init(&su->rb, buffer_len);
    su->dma_size = dma_len;
    su->dma_buf = malloc(buffer_len);

    su->lock = osSemaphoreNew(1, 0, NULL);
    vQueueAddToRegistry(su->lock, "su_lock");
    osSemaphoreRelease(su->lock);
}

void su_xmit(safe_uart_t * su)
{
	uint8_t * tmp;
	uint32_t readed = rb_read(&su->rb, su->dma_size, &tmp);

	memcpy(su->dma_buf, tmp, readed);
	HAL_StatusTypeDef res = HAL_UART_Transmit_DMA(su->uart, su->dma_buf, readed);
	if (res != HAL_OK)
	{
		INFO("HAL_UART_Transmit_DMA res=%02X", res);
	}
}

void su_write(safe_uart_t * su, uint8_t * data, uint32_t len)
{
	if (!rb_write(&su->rb, len, data))
		WARN("TX RB full");

	if (xPortIsInsideInterrupt())
	{
		if (osSemaphoreGetCount(su->lock) == 0)
		{
			//no transfer active
			if (HAL_DMA_GetState(su->uart->hdmatx) == HAL_DMA_STATE_READY
					&& su->uart->gState == HAL_UART_STATE_READY)
			{
		        su_xmit(su);
			}
		}
	}
	else
	{
		//lock this section to prevent race condition
		osSemaphoreAcquire(su->lock, WAIT_INF);

		//no transfer active
		if (HAL_DMA_GetState(su->uart->hdmatx) == HAL_DMA_STATE_READY
				&& su->uart->gState == HAL_UART_STATE_READY)
		{
	        su_xmit(su);
		}

		osSemaphoreRelease(su->lock);
	}
}

void su_clear(safe_uart_t * su)
{
    rb_clear(&su->rb);
}

//inside the interrupt (HAL_UART_STATE_READY is set in this context)
void su_tx_done(safe_uart_t * su)
{
    if (rb_length(&su->rb) > 0)
    {
        su_xmit(su);
    }
}
