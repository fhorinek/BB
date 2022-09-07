/*
 * flightbook.h
 *
 *  Created on: Feb 27, 2022
 *      Author: tilmann@bubecks.de
 */

#ifndef GUI_FLIGHTBOOK_H_
#define GUI_FLIGHTBOOK_H_

#include "gui/gui.h"

DECLARE_TASK(flightbook);

extern char flightbook_flight_filename[PATH_LEN];
void flightbook_flights_open_fm(bool anim);

#endif /* GUI_FLIGHTBOOK_H_ */
