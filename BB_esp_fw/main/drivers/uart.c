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

#define UART_RX_BUFF	256
#define UART_TX_BUFF	256

#define STREAM_RX_BUFFER_SIZE	128
static stream_t uart_stream;
static uint8_t stream_rx_buffer[STREAM_RX_BUFFER_SIZE];

void uart_send(uint8_t *data, uint16_t len)
{
    uart_write_bytes(UART_PORT, data, len);
}

int uart_elog_vprintf(const char *format, ...)
{
    va_list arp;
    uint8_t msg_buff[2048];
    uint16_t length;

    msg_buff[0] = 0xFF;

    va_start(arp, format);
    length = vsnprintf((char*) msg_buff + 1, sizeof(msg_buff) - 1, format, arp);
    va_end(arp);

    //omit ending \n
    protocol_send(PROTO_DEBUG, msg_buff, length);

    return 0;
}

static QueueHandle_t uart_queue;

static void uart_event_task(void *pvParameters)
{
    uart_event_t event;
    uint8_t rx_buff[UART_RX_BUFF];

    for (;;)
    {
        //Waiting for UART event.
        if (xQueueReceive(uart_queue, (void*) &event, WAIT_INF))
        {
            switch (event.type)
            {
                case UART_DATA:
                {
                    uart_read_bytes(UART_PORT, rx_buff, event.size, WAIT_INF);
                    for (uint16_t i = 0; i < event.size; i++)
                    {
                        bool have_data = stream_parse(&uart_stream, rx_buff[i]);
                        if (have_data)
                        {
                            protocol_handle(uart_stream.buffer, uart_stream.lenght);
                        }
                    }
                }
                break;
                case UART_FIFO_OVF:
                    ERR("hw fifo overflow");
                    uart_flush_input(UART_PORT);
                    xQueueReset(uart_queue);
                break;
                case UART_BUFFER_FULL:
                    ERR("ring buffer full");
                    uart_flush_input(UART_PORT);
                    xQueueReset(uart_queue);
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

    vTaskDelete(NULL);
}

void uart_init()
{
    stream_init(&uart_stream, stream_rx_buffer, STREAM_RX_BUFFER_SIZE);

    uart_config_t uart_config = {
        .baud_rate = UART_BAUDRATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };

    uart_driver_install(UART_PORT, UART_RX_BUFF, UART_TX_BUFF, 20, &uart_queue, ESP_INTR_FLAG_IRAM);
    uart_param_config(UART_PORT, &uart_config);
    uart_set_pin(UART_PORT, UART_TX_PIN, UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    xTaskCreate(uart_event_task, "uart_event_task", 2048, NULL, 12, NULL);

//    esp_log_set_vprintf((vprintf_like_t)uart_elog_vprintf);
}
