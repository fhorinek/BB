/*
 * uart.c
 *
 *  Created on: 3. 12. 2020
 *      Author: horinek
 */

#define DEBUG_LEVEL DBG_DEBUG

#include "../../main/drivers/uart.h"

#include <driver/uart.h>

#include "../../main/etc/stream.h"
#include "../../main/protocol.h"

#define UART_PORT		UART_NUM_0
#define UART_TX_PIN		GPIO_NUM_1
#define UART_RX_PIN		GPIO_NUM_3

#define UART_RX_BUFF	1024
#define UART_TX_BUFF	1024

#define STREAM_RX_BUFFER_SIZE	256
static stream_t uart_stream;

void uart_send(uint8_t *data, uint16_t len)
{
    uart_write_bytes(UART_PORT, (char *)data, len);
}

int uart_elog_vprintf(const char *format, va_list args)
{
    static uint8_t msg_buff[256];
    uint16_t length;

    length = vsnprintf((char*) msg_buff + 1, sizeof(msg_buff) - 1, format, args);

    uint8_t type = PROTO_NA;

    switch (msg_buff[1])
    {
		case ('D'): type = DBG_DEBUG; break;
		case ('I'): type = DBG_INFO; break;
		case ('W'): type = DBG_WARN; break;
		case ('E'): type = DBG_ERROR; break;
    }

    if (type == PROTO_NA || length < 2)
    {
    	msg_buff[0] = type;
    	protocol_send(PROTO_DEBUG, msg_buff, length);
    }
    else
    {
    	msg_buff[2] = type;
    	protocol_send(PROTO_DEBUG, msg_buff + 2, length - 2);
    }

    return length;
}

static QueueHandle_t uart_queue;

static void uart_event_task(void *pvParameters)
{
    uart_event_t event;

    uint8_t * uart_rx_buff = ps_malloc(UART_RX_BUFF);

    for (;;)
    {
        //Waiting for UART event.
        if (xQueueReceive(uart_queue, (void*) &event, WAIT_INF))
        {
            switch (event.type)
            {
                case UART_DATA:
                {
                    uart_read_bytes(UART_PORT, uart_rx_buff, event.size, WAIT_INF);
                    for (uint16_t i = 0; i < event.size; i++)
                    {
                        bool have_data = stream_parse(&uart_stream, uart_rx_buff[i]);
                        if (have_data)
                        {
                            protocol_handle(uart_stream.packet_type, uart_stream.buffer, uart_stream.lenght);
                        }
                    }
                }
                break;
                case UART_FIFO_OVF:
                    ERR("hw fifo overflow");
                    uart_flush_input(UART_PORT);
                    xQueueReset(uart_queue);
                    stream_reset(&uart_stream);
                break;
                case UART_BUFFER_FULL:
                    ERR("ring buffer full");
                    uart_flush_input(UART_PORT);
                    xQueueReset(uart_queue);
                    stream_reset(&uart_stream);
                break;
                case UART_BREAK:
                    ERR("uart rx break");
                break;
                case UART_PARITY_ERR:
                    ERR("uart parity error");
                break;
                case UART_FRAME_ERR:
                    ERR("uart frame error");
                break;
                    //Others
                default:
                    ERR("uart event type: %d", event.type);
                break;
            }
        }
    }

    free(uart_rx_buff);

    vTaskDelete(NULL);
}


void uart_init()
{
	uint8_t * stream_rx_buffer = ps_malloc(STREAM_RX_BUFFER_SIZE);
    stream_init(&uart_stream, stream_rx_buffer, STREAM_RX_BUFFER_SIZE);

    uart_config_t uart_config = {
        .baud_rate = UART_BAUDRATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };

    uart_driver_install(UART_PORT, UART_RX_BUFF, UART_TX_BUFF, 20, &uart_queue, 0);
    uart_param_config(UART_PORT, &uart_config);
    uart_set_pin(UART_PORT, UART_TX_PIN, UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    esp_log_set_vprintf((vprintf_like_t)uart_elog_vprintf);

    xTaskCreate(uart_event_task, "uart_event_task", 1024 * 3, NULL, 16, NULL);

}
