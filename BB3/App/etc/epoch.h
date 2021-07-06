/*
 * epoch.h
 *
 *  Created on: May 8, 2020
 *      Author: horinek
 */

#ifndef ETC_EPOCH_H_
#define ETC_EPOCH_H_

#include "../common.h"

uint64_t datetime_to_epoch(uint8_t sec, uint8_t min, uint8_t hour, uint8_t day, uint8_t month, uint16_t year);
void datetime_from_epoch(uint64_t epoch, uint8_t * psec, uint8_t * pmin, uint8_t * phour, uint8_t * pday, uint8_t * pwday, uint8_t * pmonth, uint16_t * pyear);
uint8_t datetime_wday_from_epoch(uint64_t epoch);
uint8_t datetime_number_of_days(uint8_t month, uint16_t year);
void time_from_epoch(uint32_t epoch, uint8_t * psec, uint8_t * pmin, uint8_t * phour);

#endif /* ETC_EPOCH_H_ */
