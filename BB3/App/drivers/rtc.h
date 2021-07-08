/*
 * rtc.h
 *
 *  Created on: Sep 25, 2020
 *      Author: horinek
 */

#ifndef DRIVERS_RTC_H_
#define DRIVERS_RTC_H_

#include "../common.h"

void rtc_init();

bool rtc_is_waiting_or_valid();
bool rtc_is_valid();

void rtc_flag_set();
void rtc_flag_wait();

void rtc_get_time(uint8_t * hour, uint8_t * minute, uint8_t * second);
void rtc_set_time(uint8_t hour, uint8_t minute, uint8_t second);

void rtc_get_date(uint8_t * day, uint8_t * wday, uint8_t * month, uint16_t * year);
void rtc_set_date(uint8_t day, uint8_t wday, uint8_t month, uint16_t year);
uint64_t rtc_get_epoch();

#endif /* DRIVERS_RTC_H_ */
