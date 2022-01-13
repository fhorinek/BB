/*
 * wind2.h
 *
 *  Created on: Jan 23, 2017
 *      Author: fiala
 */

#ifndef WIND_H_
#define WIND_H_

#include "common.h"

#define WIND_NUM_OF_SECTORS 8

struct wind_data_t
{
	float 		dir[WIND_NUM_OF_SECTORS];
	float 		spd[WIND_NUM_OF_SECTORS];
	uint8_t 	old_sector;
	int8_t		sectors_cnt;

	//calculated wind values
	bool 		valid;		// was wind calculated?
	uint32_t    valid_from;

	float 		speed;		// m/s
	float 		direction;	// degrees
};


void wind_init(void);
void wind_step(void);


#endif /* WIND_H_ */
