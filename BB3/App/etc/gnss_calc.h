/*
 * gnss_calc.h
 *
 *  Created on: May 29, 2020
 *      Author: horinek
 */

#ifndef FC_GNSS_CALC_H_
#define FC_GNSS_CALC_H_

#include "../common.h"

void gnss_destination(float lat1, float lon1, float angle, float distance_km, float * lat2, float * lon2);
uint32_t gnss_distance(int32_t lat1, int32_t lon1, int32_t lat2, int32_t lon2, bool FAI, int16_t * bearing);

#endif /* FC_GNSS_CALC_H_ */
