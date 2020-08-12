/*
 * widget_dummy.c
 *
 *  Created on: 11. 8. 2020
 *      Author: horinek
 */

#include "widget_dummy.h"

#include "widget.h"

widget_t widget_dummy =
{
		"Dummy widget",
		"dummy",
		widget_init,
		widget_stop,
		NULL,
		NULL,
};
