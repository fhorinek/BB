/*
 * timezone.c
 *
 *  Created on: Sep 25, 2020
 *      Author: horinek
 */

#include "timezone.h"

//offset from UTC in sec / 100

int16_t timezone_offset[] = {
	-432,	//UTC_n1200
	-396,	//UTC_n1100
	-360,	//UTC_n1000
	-342,	//UTC_n0930
	-324,	//UTC_n0900
	-288,	//UTC_n0800
	-252,	//UTC_n0700
	-216,	//UTC_n0600
	-180,	//UTC_n0500
	-144,	//UTC_n0400
	-126,	//UTC_n0330
	-108,	//UTC_n0300
	-72,	//UTC_n0200
	-36,	//UTC_n0100
	0,	    //UTC_p0000
	36,	    //UTC_p0100
	72,	    //UTC_p0200
	108,	//UTC_p0300
	126,	//UTC_p0330
	144,	//UTC_p0400
	162,	//UTC_p0430
	180,	//UTC_p0500
	198,	//UTC_p0530
	207,	//UTC_p0545
	216,	//UTC_p0600
	234,	//UTC_p0630
	252,	//UTC_p0700
	288,	//UTC_p0800
	315,	//UTC_p0845
	324,	//UTC_p0900
	342,	//UTC_p0930
	360,	//UTC_p1000
	378,	//UTC_p1030
	396,	//UTC_p1100
	432,	//UTC_p1200
	459,	//UTC_p1245
	468,	//UTC_p1300
	504,	//UTC_p1400
};

int32_t timezone_get_offset(uint8_t index)
{
	return timezone_offset[index] * 100;
}
