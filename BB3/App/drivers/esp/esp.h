/*
 * esp.h
 *
 *  Created on: Dec 3, 2020
 *      Author: horinek
 */

#ifndef DRIVERS_ESP_ESP_H_
#define DRIVERS_ESP_ESP_H_

#include "../../common.h"

void esp_init();
void esp_deinit();
void esp_step();
void esp_reset();

void esp_enable_external_programmer();
void esp_disable_external_programmer();

void thread_esp_start(void * argument);
void thread_esp_spi_start(void * argument);

uint16_t esp_read_bytes(uint8_t * data, uint16_t len, uint32_t timeout);

#endif /* DRIVERS_ESP_ESP_H_ */
