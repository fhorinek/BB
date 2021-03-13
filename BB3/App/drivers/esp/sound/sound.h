/*
 * sound.h
 *
 *  Created on: Jan 20, 2021
 *      Author: horinek
 */

#ifndef DRIVERS_ESP_SOUND_SOUND_H_
#define DRIVERS_ESP_SOUND_SOUND_H_

#include "common.h"

void sound_start(char * filename);
void sound_read_next(uint8_t id, uint32_t size);

void sound_close();
void sound_stop();

#endif /* DRIVERS_ESP_SOUND_SOUND_H_ */
