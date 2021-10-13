/*
 * gnss.h
 *
 *  Created on: Oct 7, 2021
 *      Author: horinek
 */

#ifndef FC_TELEMETRY_GNSS_H_
#define FC_TELEMETRY_GNSS_H_

#include "common.h"

bool gnss_rmc_msg(char * buff, uint16_t len);
bool gnss_gga_msg(char * buff, uint16_t len);

#endif /* FC_TELEMETRY_GNSS_H_ */
