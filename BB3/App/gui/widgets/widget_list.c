/*
 * widget_list.c
 *
 *  Created on: 11. 8. 2020
 *      Author: horinek
 */

#include "widgets.h"

DECLARE_WIDGET(Vario);
DECLARE_WIDGET(Avg);
DECLARE_WIDGET(Bar);
DECLARE_WIDGET(VarioGraph);
DECLARE_WIDGET(FTime);
DECLARE_WIDGET(Alt1);
DECLARE_WIDGET(Alt2);
DECLARE_WIDGET(HeightTO);
DECLARE_WIDGET(CompHdg);
DECLARE_WIDGET(CompArrow);
DECLARE_WIDGET(CompPoints);
DECLARE_WIDGET(Map);
DECLARE_WIDGET(GSpeed);
DECLARE_WIDGET(GAlt);
DECLARE_WIDGET(GHdg);
DECLARE_WIDGET(GHdgArrow);
DECLARE_WIDGET(GHdgPoints);
DECLARE_WIDGET(Glide);
DECLARE_WIDGET(Battery);
DECLARE_WIDGET(Agl);
DECLARE_WIDGET(Odo);
DECLARE_WIDGET(Acc);
DECLARE_WIDGET(TAss);
DECLARE_WIDGET(TTime);
DECLARE_WIDGET(TGain);
DECLARE_WIDGET(DToff);
DECLARE_WIDGET(HdgToff);
DECLARE_WIDGET(WindDir);
DECLARE_WIDGET(WindSpd);
DECLARE_WIDGET(TTrace);
DECLARE_WIDGET(WindDirPoints);
DECLARE_WIDGET(TimeDate);

widget_t * widgets[] =
{
	// Insert new widget in a meaningfull order, as this list is presented to user:
    LIST_WIDGET(Vario),
    LIST_WIDGET(Avg),
	LIST_WIDGET(HeightTO),
    LIST_WIDGET(Glide),
    LIST_WIDGET(Bar),
    LIST_WIDGET(VarioGraph),
    LIST_WIDGET(Alt1),
    LIST_WIDGET(Alt2),
    LIST_WIDGET(Agl),
    LIST_WIDGET(FTime),
    LIST_WIDGET(CompHdg),
    LIST_WIDGET(CompArrow),
    LIST_WIDGET(CompPoints),
    LIST_WIDGET(GSpeed),
    LIST_WIDGET(GAlt),
	LIST_WIDGET(GHdg),
	LIST_WIDGET(GHdgPoints),
	LIST_WIDGET(GHdgArrow),
	LIST_WIDGET(Battery),
	LIST_WIDGET(Odo),
	LIST_WIDGET(Acc),
    LIST_WIDGET(TTrace),
    LIST_WIDGET(TAss),
    LIST_WIDGET(TTime),
    LIST_WIDGET(TGain),
    LIST_WIDGET(DToff),
	LIST_WIDGET(HdgToff),
	LIST_WIDGET(WindDir),
    LIST_WIDGET(WindSpd),
    LIST_WIDGET(WindDirPoints),
    LIST_WIDGET(TimeDate),

//    LIST_WIDGET(Map),
};


uint8_t number_of_widgets()
{
	return sizeof(widgets) / sizeof(widget_t *);
}

