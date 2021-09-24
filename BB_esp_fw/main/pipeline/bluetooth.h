/*
 * bluetooth.h
 *
 *  Created on: 19. 1. 2021
 *      Author: horinek
 */

#ifndef MAIN_PIPELINE_BLUETOOTH_H_
#define MAIN_PIPELINE_BLUETOOTH_H_

#include "pipeline.h"

void pipe_bluetooth_init();

void pipe_bluetooth_event(audio_event_iface_msg_t * msg);

#endif /* MAIN_PIPELINE_BLUETOOTH_H_ */
