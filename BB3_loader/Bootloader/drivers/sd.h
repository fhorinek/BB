/*
 * sd.h
 *
 *  Created on: May 5, 2020
 *      Author: horinek
 */

#ifndef DRIVERS_SD_H_
#define DRIVERS_SD_H_

#include "../common.h"

bool sd_detect();

void sd_init();
void sd_deinit();

bool sd_mount();
void sd_unmount();

#endif /* DRIVERS_SD_H_ */
