/*
 * bq25895.h
 *
 *  Created on: Oct 12, 2020
 *      Author: John
 */

#ifndef DRIVERS_BQ25895_H_
#define DRIVERS_BQ25895_H_

#include "common.h"

void bq25895_init();
void bq25895_step();
void bq25895_boost_voltage(uint8_t val);

#endif /* DRIVERS_BQ25895_H_ */
