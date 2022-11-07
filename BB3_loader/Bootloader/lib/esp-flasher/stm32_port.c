/* Copyright 2020 Espressif Systems (Shanghai) PTE LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define DEBUG_LEVEL DBG_DEBUG

#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/param.h>
#include <stdio.h>

#include "stm32_port.h"
#include "flasher.h"

#include "common.h"

//#define SERIAL_DEBUG_ENABLE

#ifdef SERIAL_DEBUG_ENABLE

static void serial_debug_print(const uint8_t *data, uint16_t size, bool write)
{
    static bool write_prev = false;

    if(write_prev != write) {
        write_prev = write;
        DBG("\n--- %s ---", write ? "WRITE" : "READ");
    }

    if (size)
        DUMP(data, size);
    else
        DBG(" Timeout");
}

#else

static void serial_debug_print(const uint8_t *data, uint16_t size, bool write) { }

#endif

static uint32_t s_time_end;

esp_loader_error_t loader_port_serial_write(const uint8_t *data, uint16_t size, uint32_t timeout)
{
    serial_debug_print(data, size, true);

    HAL_StatusTypeDef err = HAL_UART_Transmit(esp_uart, (uint8_t *)data, size, timeout);

    if (err == HAL_OK) {
        return ESP_LOADER_SUCCESS;
    } else if (err == HAL_TIMEOUT) {
        return ESP_LOADER_ERROR_TIMEOUT;
    } else {
        return ESP_LOADER_ERROR_FAIL;
    }
}


esp_loader_error_t loader_port_serial_read(uint8_t *data, uint16_t size, uint32_t timeout)
{
    memset(data, 0x22, size);

    uint16_t readed = esp_read_bytes(data, size, timeout);

    serial_debug_print(data, size, false);

    if (readed == size) {
        return ESP_LOADER_SUCCESS;
    } else {
        return ESP_LOADER_ERROR_TIMEOUT;
    }

//    HAL_StatusTypeDef err = HAL_UART_Receive(esp_uart, data, size, timeout);
//
//    serial_debug_print(data, size, false);
//
//    if (err == HAL_OK) {
//        return ESP_LOADER_SUCCESS;
//    } else if (err == HAL_TIMEOUT) {
//        return ESP_LOADER_ERROR_TIMEOUT;
//    } else {
//        return ESP_LOADER_ERROR_FAIL;
//    }
}

// Set GPIO0 LOW, then
// assert reset pin for 100 milliseconds.
void loader_port_enter_bootloader(void)
{
    GpioWrite(ESP_EN, LOW);
    GpioWrite(ESP_BOOT, LOW);
    HAL_Delay(50);
    GpioWrite(ESP_EN, HIGH);
    HAL_Delay(50);
    GpioWrite(ESP_BOOT, HIGH);
}


void loader_port_reset_target(void)
{
    GpioWrite(ESP_EN, LOW);
    HAL_Delay(50);
    GpioWrite(ESP_BOOT, HIGH);
}


void loader_port_delay_ms(uint32_t ms)
{
    HAL_Delay(ms);
}


void loader_port_start_timer(uint32_t ms)
{
    s_time_end = HAL_GetTick() + ms;
}


uint32_t loader_port_remaining_time(void)
{
    int32_t remaining = s_time_end - HAL_GetTick();
    return (remaining > 0) ? (uint32_t)remaining : 0;
}


void loader_port_debug_print(const char *str)
{
    DBG("ESP flasher: %s", str);
}

esp_loader_error_t loader_port_change_baudrate(uint32_t baudrate)
{
    esp_uart->Init.BaudRate = baudrate;

    if( HAL_UART_Init(esp_uart) != HAL_OK )
    {
        return ESP_LOADER_ERROR_FAIL;
    }

    return ESP_LOADER_SUCCESS;
}
