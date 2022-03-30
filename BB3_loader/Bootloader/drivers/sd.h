/*
 * sd.h
 *
 *  Created on: May 5, 2020
 *      Author: horinek
 */

#ifndef DRIVERS_SD_H_
#define DRIVERS_SD_H_

#include "../common.h"

extern lfs_t lfs;

void sd_init();
bool sd_mount();
void sd_unmount();

void sd_format();


#endif /* DRIVERS_SD_H_ */
