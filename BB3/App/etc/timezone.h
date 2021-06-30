/*
 * timezone.h
 *
 *  Created on: Sep 25, 2020
 *      Author: horinek
 */

#ifndef ETC_TIMEZONE_H_
#define ETC_TIMEZONE_H_

#include "../common.h"

#define NUMBER_OF_TIMEZONES    38

#define	UTC_n1200	0
#define	UTC_n1100	1
#define	UTC_n1000	2
#define	UTC_n0930	3
#define	UTC_n0900	4
#define	UTC_n0800	5
#define	UTC_n0700	6
#define	UTC_n0600	7
#define	UTC_n0500	8
#define	UTC_n0400	9
#define	UTC_n0330	10
#define	UTC_n0300	11
#define	UTC_n0200	12
#define	UTC_n0100	13
#define	UTC_p0000	14
#define	UTC_p0100	15
#define	UTC_p0200	16
#define	UTC_p0300	17
#define	UTC_p0330	18
#define	UTC_p0400	19
#define	UTC_p0430	20
#define	UTC_p0500	21
#define	UTC_p0530	22
#define	UTC_p0545	23
#define	UTC_p0600	24
#define	UTC_p0630	25
#define	UTC_p0700	26
#define	UTC_p0800	27
#define	UTC_p0845	28
#define	UTC_p0900	29
#define	UTC_p0930	30
#define	UTC_p1000	31
#define	UTC_p1030	32
#define	UTC_p1100	33
#define	UTC_p1200	34
#define	UTC_p1245	35
#define	UTC_p1300	36
#define	UTC_p1400	37

int32_t timezone_get_offset(uint8_t index, bool dst);

#endif /* ETC_TIMEZONE_H_ */
