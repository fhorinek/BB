/*
 * csv_logger.h
 *
 *  Created on: Jan 19, 2022
 *      Author: thrull
 */

#ifndef FC_LOGGER_CSV_H_
#define FC_LOGGER_CSV_H_

#include "common.h"
#include "fc/fc.h"

void csv_init();
void csv_start();
void csv_stop();

fc_logger_status_t csv_logger_state();

#endif /* FC_LOGGER_CSV_H_ */
