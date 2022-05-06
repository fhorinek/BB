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
#define PSRAM_SIZE		(8 * 1024 * 1024)

void PSRAM_init();
bool PSRAM_test();

#define ps_malloc(size) ps_malloc_real(size, __FILE__, __LINE__)

void * ps_malloc_real(uint32_t requested_size, char * name, uint32_t lineno);
void * ps_realloc(void * ptr, uint32_t size);
void ps_free(void * ptr);

void ps_malloc_init();
void ps_malloc_info();

#endif /* DRIVERS_PSRAM_H_ */
