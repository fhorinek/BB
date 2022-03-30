/*
 * rev.h
 *
 *  Created on: Feb 1, 2021
 *      Author: horinek
 */

#ifndef DRIVERS_REV_H_
#define DRIVERS_REV_H_

#include "common.h"

void rev_get_uuid(uint8_t * buff);
uint32_t rev_get_short_id();

uint32_t rev_get_build_number();
void rev_get_sw_string(char * str);

uint8_t rev_get_hw();

#endif /* DRIVERS_REV_H_ */
