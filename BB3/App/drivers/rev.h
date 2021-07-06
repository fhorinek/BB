/*
 * rev.h
 *
 *  Created on: Feb 1, 2021
 *      Author: horinek
 */

#ifndef DRIVERS_REV_H_
#define DRIVERS_REV_H_

#include "common.h"

uint32_t rew_get_sw();
void rew_get_sw_string(char * str);

uint8_t rev_get_hw();

#endif /* DRIVERS_REV_H_ */
