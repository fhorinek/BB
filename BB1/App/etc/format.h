/*
 * format.h
 *
 *  Created on: 27. 5. 2020
 *      Author: horinek
 */

#ifndef ETC_FORMAT_H_
#define ETC_FORMAT_H_

#include "../common.h"

void format_gnss_datum(char * slat, char * slon, int32_t lat, int32_t lot);
void format_distance(char * buf, float in);

#endif /* ETC_FORMAT_H_ */
