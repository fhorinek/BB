/*
 * gnss_ublox_m8.h
 *
 *  Created on: 4. 5. 2020
 *      Author: horinek
 */

#ifndef DRIVERS_GNSS_UBLOX_M8_H_
#define DRIVERS_GNSS_UBLOX_M8_H_

#include "../common.h"

void ublox_init();
void ublox_deinit();
void ublox_step();

#endif /* DRIVERS_GNSS_UBLOX_M8_H_ */
