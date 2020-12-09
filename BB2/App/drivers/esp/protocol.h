/*
 * protocol.h
 *
 *  Created on: Dec 4, 2020
 *      Author: horinek
 */

#ifndef DRIVERS_ESP_PROTOCOL_H_
#define DRIVERS_ESP_PROTOCOL_H_

#include "../../common.h"

void esp_send_ping();

void protocol_send(uint8_t type, uint8_t * data, uint16_t data_len);
void protocol_handle(uint8_t * data, uint16_t len);

void esp_set_volume(uint8_t vol);

#endif /* DRIVERS_ESP_PROTOCOL_H_ */
