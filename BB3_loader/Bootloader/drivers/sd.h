/*
 * sd.h
 *
 *  Created on: May 5, 2020
 *      Author: horinek
 */

#ifndef DRIVERS_SD_H_
#define DRIVERS_SD_H_

#include "../common.h"

bool sd_mount();
void sd_unmount();

void sd_format();
void sd_set_disk_label();

#endif /* DRIVERS_SD_H_ */
