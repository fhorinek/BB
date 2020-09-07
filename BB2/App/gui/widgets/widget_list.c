/*
 * widget_list.c
 *
 *  Created on: 11. 8. 2020
 *      Author: horinek
 */

#include "widgets.h"

DECLARE_WIDGET(dummy);
DECLARE_WIDGET(flight_time);


widget_t * widgets[] =
{
		&widget_dummy,
		&widget_flight_time,
};

uint8_t number_of_widgets()
{
	return sizeof(widgets) / sizeof(widget_t *);
}

