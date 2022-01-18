/*
 * output.h
 *
 *  Created on: 19. 1. 2021
 *      Author: horinek
 */

#ifndef MAIN_PIPELINE_OUTPUT_H_
#define MAIN_PIPELINE_OUTPUT_H_

#include "pipeline.h"

void pipe_output_init();
void pipe_output_set_volume(uint8_t ch, uint8_t volume);

#endif /* MAIN_PIPELINE_OUTPUT_H_ */
