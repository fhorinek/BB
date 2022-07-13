/*
 * logger.h
 *
 *  Created on: Jun 30, 2021
 *      Author: horinek
 */

#ifndef FC_LOGGER_LOGGER_H_
#define FC_LOGGER_LOGGER_H_

#include "common.h"
#include "fc/fc.h"

void logger_init();
void logger_start();
void logger_comment(const char *format, ...);
void logger_stop();

fc_logger_status_t logger_state();

#endif /* FC_LOGGER_LOGGER_H_ */
