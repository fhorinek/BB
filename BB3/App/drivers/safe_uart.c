/*
 * safe_uart.c
 *
 *  Created on: 21. 2. 2022
 *      Author: horinek
 */

#define DEBUG_LEVEL     DBG_DEBUG

#include "safe_uart.h"
#include "FreeRTOS.h"

#define IS_IRQ_MODE()             (__get_IPSR() != 0U)

void su_init(safe_uart_t * su, UART_HandleTypeDef * uart, uint16_t queue_len)
{
    su->last_data = NULL;
    su->uart = uart;
    su->queue = xQueueCreate(queue_len, sizeof(safe_uart_item_t));
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

    if (HAL_UART_GetState(su->uart) == HAL_UART_STATE_READY)
    {
        DBG("su_write");
        su_xmit(su, ptr, len);
    }
    else
    {
        safe_uart_item_t su_item;

        su_item.data = ptr;
        su_item.len = len;


        if (IS_IRQ_MODE())
        {
            if (urgent)
                xQueueSendToFrontFromISR(su->queue, &su_item, NULL);
            else
                xQueueSendToBackFromISR(su->queue, &su_item, NULL);
        }
        else
        {
            if (urgent)
                xQueueSendToFront(su->queue, &su_item, WAIT_INF);
            else
                xQueueSendToBack(su->queue, &su_item, WAIT_INF);
        }
    }
}

void su_clear(safe_uart_t * su)
{
    xQueueReset(su->queue);
}

void su_tx_done(safe_uart_t * su)
{
    if (su->last_data != NULL)
    {
        free(su->last_data);
        su->last_data = NULL;
    }

    if (!xQueueIsQueueEmptyFromISR(su->queue))
    {
        safe_uart_item_t su_item;

        xQueueReceiveFromISR(su->queue, &su_item, NULL);

        DBG("su_tx_done");
        su_xmit(su, su_item.data, su_item.len);
    }
}
