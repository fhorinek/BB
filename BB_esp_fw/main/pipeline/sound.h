/*
 * bluetooth.h
 *
 *  Created on: 19. 1. 2021
 *      Author: horinek
 */

#ifndef MAIN_PIPELINE_SOUND_H_
#define MAIN_PIPELINE_SOUND_H_

#include "pipeline.h"

void pipe_sound_init();
void pipe_sound_event(audio_event_iface_msg_t * msg);

void pipe_sound_start(uint8_t id, uint8_t type, uint32_t len);
void pipe_sound_write(uint8_t id, uint8_t * buf, uint16_t len);

void pipe_sound_reset_request();

void pipe_sound_reset();
void pipe_sound_stop();

#endif /* MAIN_PIPELINE_SOUND_H_ */
