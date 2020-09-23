/*
 * ms5611.h
 *
 *  Created on: Sep 17, 2020
 *      Author: horinek
 */

#ifndef DRIVERS_MS5611_H_
#define DRIVERS_MS5611_H_

#include "../common.h"


//registers of the device
#define MS5611_D1		0x40
#define MS5611_D2		0x50
#define MS5611_RESET	0x1E
#define MS5611_READ		0x00
#define MS5611_PROM 0xA2 // by adding ints from 0 to 6 we can read all the prom configuration values.

// OSR (Over Sampling Ratio) constants
#define MS5611_OSR_256 	0x00
#define MS5611_OSR_512 	0x02
#define MS5611_OSR_1024 0x04
#define MS5611_OSR_2048 0x06
#define MS5611_OSR_4096 0x08

#define MS5611_RESET	0x1E

#define MS5611_ADDR	(0x77 << 1)

#define MS5611_PRESS_OSR	MS5611_OSR_4096
#define MS5611_TEMP_OSR		MS5611_OSR_256

void ms5611_Init();
void ms5611_StartTemperature();
void ms5611_ReadTemperature();
void ms5611_CompensateTemperature();
void ms5611_StartPressure();
void ms5611_ReadPressure();
float ms5611_CompensatePressure();

#endif /* DRIVERS_MS5611_H_ */
