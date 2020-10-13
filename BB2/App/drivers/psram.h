/*
 * psram.h
 *
 *  Created on: Sep 3, 2020
 *      Author: horinek
 */

#ifndef DRIVERS_PSRAM_H_
#define DRIVERS_PSRAM_H_

#include "../common.h"

#define PSRAM_ADDR		((__IO uint8_t *)OCTOSPI1_BASE)
#define PSRAM_SIZE		(16 * 1024 * 1024)

void PSRAM_Init();

#endif /* DRIVERS_PSRAM_H_ */
