/*
 * protocol.h
 *
 *  Created on: Dec 4, 2020
 *      Author: horinek
 */

#ifndef DRIVERS_ESP_PROTOCOL_H_
#define DRIVERS_ESP_PROTOCOL_H_

#include "common.h"
#include "protocol_def.h"
#include "fc/fc.h"

void esp_send_ping();

void esp_set_volume(uint8_t vol);

void esp_sound_start(uint8_t id, uint8_t type, uint32_t size);
void esp_sound_stop();

void protocol_send(uint8_t type, uint8_t * data, uint16_t data_len);
void protocol_handle(uint8_t type, uint8_t * data, uint16_t len);

uint16_t esp_spi_send(uint8_t * data, uint16_t len);
uint8_t * esp_spi_acquire_buffer_ptr(uint16_t * size_avalible);
void esp_spi_release_buffer(uint16_t data_written);

void spi_start_transfer(uint16_t size_to_read);
void esp_spi_prepare();
void esp_wifi_start_scan(wifi_list_update_cb cb);
void esp_wifi_stop_scan();

#endif /* DRIVERS_ESP_PROTOCOL_H_ */
