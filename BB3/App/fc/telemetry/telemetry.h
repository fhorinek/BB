/*
 * protocol.h
 *
 *  Created on: Oct 4, 2021
 *      Author: horinek
 */

#ifndef FC_TELEMETRY_TELEMETRY_H_
#define FC_TELEMETRY_TELEMETRY_H_

#include "common.h"

typedef enum
{
	tele_lk8ex1,
	tele_openvario,
	tele_none
} telemetry_type_t;

void telemetry_init();
void telemetry_cb();
void telemetry_start();
void telemetry_stop();

#endif /* FC_TELEMETRY_TELEMETRY_H_ */
