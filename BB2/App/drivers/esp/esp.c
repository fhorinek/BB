/*
 * esp.c
 *
 *  Created on: Dec 3, 2020
 *      Author: horinek
 */

#define DEBUG_LEVEL    DBG_DEBUG

#include "esp.h"

#include "../fc/fc.h"
#include "../etc/stream.h"

#include "protocol.h"

//DMA buffer
#define ESP_DMA_BUFFER_SIZE   512
static uint8_t esp_rx_buffer[ESP_DMA_BUFFER_SIZE];

#define ESP_STREAM_BUFFER_SIZE   256
static uint8_t esp_stream_buffer[ESP_STREAM_BUFFER_SIZE];
static stream_t esp_stream;

void esp_parser(uint8_t * data, uint16_t len, stream_result_t res);

void esp_init()
{
	fc.esp.mode = esp_starting;

	HAL_GPIO_WritePin(ESP_RESET, HIGH);
	HAL_GPIO_WritePin(ESP_BOOT_OPT, HIGH);

	HAL_UART_Receive_DMA(&esp_uart, esp_rx_buffer, ESP_DMA_BUFFER_SIZE);
	stream_init(&esp_stream, esp_stream_buffer, ESP_STREAM_BUFFER_SIZE, esp_parser);
}

void esp_deinit()
{
	fc.esp.mode = esp_off;

    HAL_UART_DeInit(&esp_uart);
	HAL_GPIO_WritePin(ESP_RESET, LOW);
}

void esp_enable_external_programmer()
{
    INFO("Enabling external programmer for ESP32");

	esp_deinit();

	fc.esp.mode = esp_external;

	//Enable esp programming
	GpioSetDirection(ESP_RESET, INPUT, GPIO_PULLUP);
	GpioSetDirection(ESP_BOOT_OPT, INPUT, GPIO_PULLUP);
}

void esp_disable_external_programmer()
{
    INFO("Disabling external programmer for ESP32");
	GpioSetDirection(ESP_RESET, OUTPUT, GPIO_PULLUP);
	GpioSetDirection(ESP_BOOT_OPT, OUTPUT, GPIO_PULLUP);
	HAL_UART_Init(&esp_uart);

	esp_init();
}

void esp_parser(uint8_t * data, uint16_t len, stream_result_t res)
{
    switch (res)
    {
        case(stream_res_message):
           protocol_handle(data, len);
        break;
        case(stream_res_dirty): //dirty string is ending with 0
           INFO(data);
        break;
        case(stream_res_error):
           DUMP(data, len);
        break;

    }



}

void esp_step()
{
    static uint16_t read_index = 0;

    uint16_t write_index = ESP_DMA_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(esp_uart.hdmarx);
    uint16_t waiting;

    //Get number of bytes waiting in buffer
    if (read_index > write_index)
    {
        waiting = ESP_DMA_BUFFER_SIZE - read_index + write_index;
    }
    else
    {
        waiting = write_index - read_index;
    }

    //parse the data
    for (uint16_t i = 0; i < waiting; i++)
    {
        if (fc.esp.mode == esp_normal || fc.esp.mode == esp_starting)
        {
            stream_parse(&esp_stream, esp_rx_buffer[read_index]);
        }
        else if (fc.esp.mode == esp_programming)
        {
            //TODD:
        }

        read_index = (read_index + 1) % ESP_DMA_BUFFER_SIZE;
    }
}


