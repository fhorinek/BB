/*
 * safe_uart.c
 *
 *  Created on: 21. 2. 2022
 *      Author: horinek
 */

//#define DEBUG_LEVEL     DBG_DEBUG

#include "safe_uart.h"
#include "FreeRTOS.h"


void su_init(safe_uart_t * su, UART_HandleTypeDef * uart, uint16_t queue_len)
{
    su->last_data = NULL;
    su->uart = uart;
    su->queue = xQueueCreate(queue_len, sizeof(safe_uart_item_t));
    su->lock = osSemaphoreNew(1, 0, NULL);
    osSemaphoreRelease(su->lock);
}

void su_xmit(safe_uart_t * su, uint8_t * data, uint32_t len)
{
    FASSERT(su->last_data == NULL);

    DBG("su_xmit");
    DUMP(data, len);
    HAL_UART_Transmit_DMA(su->uart, data, len);
    su->last_data = data;
}

void su_write(safe_uart_t * su, uint8_t * data, uint32_t len, bool urgent)
{
    uint8_t * ptr = malloc(len);
    memcpy(ptr, data, len);

    safe_uart_item_t su_item;

    su_item.data = ptr;
    su_item.len = len;

    if (IS_IRQ_MODE())
    {
        //no lock active
        if (osSemaphoreGetCount(su->lock) == 0)
        {
            //transmit now!
            su_xmit(su, su_item.data, su_item.len);
        }
        else
        {
            //put to the queue
            if (urgent)
                xQueueSendToFrontFromISR(su->queue, &su_item, NULL);
            else
                xQueueSendToBackFromISR(su->queue, &su_item, NULL);
        }
    }
    else
    {
        //lock this section to prevent race condition
        osSemaphoreAcquire(su->lock, WAIT_INF);

        //no transfer active
        if (HAL_DMA_GetState(su->uart->hdmatx) == HAL_DMA_STATE_READY
                && HAL_UART_GetState(su->uart) == HAL_UART_STATE_READY)
        {
            su_xmit(su, su_item.data, su_item.len);
        }
        else
        {
            if (urgent)
                xQueueSendToFront(su->queue, &su_item, WAIT_INF);
            else
                xQueueSendToBack(su->queue, &su_item, WAIT_INF);
        }

        osSemaphoreRelease(su->lock);
    }


}

void su_clear(safe_uart_t * su)
{
    xQueueReset(su->queue);
}

//inside the interrupt (HAL_UART_STATE_READY is set in this context)
void su_tx_done(safe_uart_t * su)
{
    DBG("su_tx_done");
    if (su->last_data != NULL)
    {
        system_free(su->last_data);
        su->last_data = NULL;
    }

    if (!xQueueIsQueueEmptyFromISR(su->queue))
    {
        safe_uart_item_t su_item;

        xQueueReceiveFromISR(su->queue, &su_item, NULL);

        su_xmit(su, su_item.data, su_item.len);
    }
}
