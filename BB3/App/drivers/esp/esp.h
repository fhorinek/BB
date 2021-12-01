/*
 * esp.h
 *
 *  Created on: Dec 3, 2020
 *      Author: horinek
 */

#ifndef DRIVERS_ESP_ESP_H_
#define DRIVERS_ESP_ESP_H_

#include "common.h"
#include "fc/fc.h"

void esp_init();
void esp_deinit();
void esp_step();
void esp_device_reset();
void esp_state_reset();

void esp_reboot();

void esp_start_dma();

void esp_enable_external_programmer(esp_mode_t prog_mode);
void esp_disable_external_programmer();

void thread_esp_start(void * argument);
void thread_esp_spi_start(void * argument);

uint16_t esp_read_bytes(uint8_t * data, uint16_t len, uint32_t timeout);

void esp_uart_rx_irq_cb();
void spi_dma_done_cb();

#endif /* DRIVERS_ESP_ESP_H_ */
