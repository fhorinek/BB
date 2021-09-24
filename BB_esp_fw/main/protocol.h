/*
 * protocol.h
 *
 *  Created on: 4. 12. 2020
 *      Author: horinek
 */

#ifndef MAIN_PROTOCOL_H_
#define MAIN_PROTOCOL_H_

#include "../main/common.h"

void protocol_enable();

void protocol_send_info();

void protocol_send_spi_req();
void protocol_send_spi_ready(uint32_t len);

void protocol_send_sound_reg_more(uint8_t id, uint32_t len);

void protocol_send(uint8_t type, uint8_t * data, uint16_t data_len);
void protocol_handle(uint8_t type, uint8_t * data, uint16_t len);

void protocol_send_heartbeat();

#endif /* MAIN_PROTOCOL_H_ */
