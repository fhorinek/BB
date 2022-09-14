/*
 * flightbook_flight_show.h
 *
 *  Created on: Feb 27, 2022
 *      Author: tilmann@bubecks.de
 */

#ifndef GUI_FLIGHTBOOK_FLIGHT_H_
#define GUI_FLIGHTBOOK_FLIGHT_H_

#include "gui/gui.h"

DECLARE_TASK(flightbook_flight);

void flightbook_flight_open(char * path, uint8_t fm_level);

#endif /* GUI_FLIGHTBOOK_FLIGHT_H_ */
