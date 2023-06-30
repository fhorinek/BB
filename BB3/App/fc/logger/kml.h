/*
 * kml.h
 *
 *  Created on: Jun 27, 2023
 *      Author: tilmann@bubecks.de
 */

#ifndef FC_LOGGER_KML_H_
#define FC_LOGGER_KML_H_

#include "common.h"
#include "fc/fc.h"

void kml_init();
void kml_start();
void kml_stop();
void kml_comment(char * text);

fc_logger_status_t kml_logger_state();

#endif /* FC_LOGGER_KML_H_ */
