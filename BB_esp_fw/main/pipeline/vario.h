/*
 * bluetooth.h
 *
 *  Created on: 19. 1. 2021
 *      Author: horinek
 */

#ifndef MAIN_PIPELINE_VARIO_H_
#define MAIN_PIPELINE_VARIO_H_

#include "pipeline.h"

typedef struct
{
	uint32_t duration;
	uint32_t pause;
	uint16_t frequency;
} vario_sound_t;

void pipe_vario_init();
void pipe_vario_event(audio_event_iface_msg_t * msg);

typedef struct _tone_part_t
{
	int16_t * buffer;
	uint16_t size;
	uint16_t repeat;
} tone_part_t;

tone_part_t * vario_create_part(uint16_t freq, uint16_t duration);
void pipe_vario_replace(tone_part_t ** new_tones, uint8_t cnt);

void pipe_vario_step();

#endif /* MAIN_PIPELINE_VARIO_H_ */
