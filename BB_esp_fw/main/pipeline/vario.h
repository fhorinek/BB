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

typedef struct
{
	int16_t tone;
	int16_t dura;
} tone_pair_t;

void vario_create_sequence(tone_pair_t * pairs, uint8_t cnt);

#define ONE_MS	((OUTPUT_SAMPLERATE / 1000) * 2)

void pipe_vario_step();
void vario_proces_packet(proto_tone_play_t * packet);

#endif /* MAIN_PIPELINE_VARIO_H_ */
