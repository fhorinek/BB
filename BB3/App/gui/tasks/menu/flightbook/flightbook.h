/*
 * flightbook.h
 *
 *  Created on: Feb 27, 2022
 *      Author: tilmann@bubecks.de
 */

#ifndef GUI_FLIGHTBOOK_H_
#define GUI_FLIGHTBOOK_H_

#include "gui/gui.h"

void flightbook_open();
bool flightbook_flights_fm_cb(uint8_t event, char * path);

#endif /* GUI_FLIGHTBOOK_H_ */
