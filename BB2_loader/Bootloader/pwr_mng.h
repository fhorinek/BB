/*
 * pwr_mng.h
 *
 *  Created on: Oct 13, 2020
 *      Author: John
 */

#ifndef PWR_MNG_H_
#define PWR_MNG_H_

#include "common.h"

#define PWR_CHARGE_NONE	    0
#define PWR_CHARGE_WEAK     1
#define PWR_CHARGE_SLOW     2
#define PWR_CHARGE_FAST	    3
#define PWR_CHARGE_QUICK    4

#define PWR_DATA_NONE	    0
#define PWR_DATA_CHARGE     1
#define PWR_DATA_ACTIVE     2

typedef struct
{
	uint8_t charge_port;
	uint8_t data_port;

	int16_t bat_current; //in mA
	uint8_t	bat_charge;	//in %
	uint16_t bat_time_to_full; //in s
	uint16_t bat_cap; //in mA

} power_mng_t;

extern power_mng_t pwr;

void pwr_init();
void pwr_step();

#endif /* PWR_MNG_H_ */
