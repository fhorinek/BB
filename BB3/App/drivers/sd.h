/*
 * sd.h
 *
 *  Created on: May 5, 2020
 *      Author: horinek
 */

#ifndef DRIVERS_SD_H_
#define DRIVERS_SD_H_

#include "../common.h"

void sd_init_failsafe();

void sd_init();
void sd_deinit();

uint8_t sd_read_blocks(uint32_t *pData, uint32_t ReadAddr, uint32_t NumOfBlocks);
uint8_t sd_write_blocks(uint32_t *pData, uint32_t ReadAddr, uint32_t NumOfBlocks);

#endif /* DRIVERS_SD_H_ */
