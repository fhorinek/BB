/*
 * widget_flags.h
 *
 *  Created on: 3. 11. 2021
 *      Author: horinek
 */

#ifndef GUI_WIDGETS_WIDGET_FLAGS_H_
#define GUI_WIDGETS_WIDGET_FLAGS_H_

typedef struct
{
	char flag;
	char * on_text;
	char * off_text;
	void * cb;
} widget_flag_def_t;

typedef enum
{
	wf_label_hide = 0,
	wf_units_hide,
	wf_decimal_precision,
    wf_alt_unit,
    wf_glide_show_avg_vario,

	wf_def_size
}  widget_flags_id_t;

#define _b(a) (1 << a)

extern widget_flag_def_t widgets_flags[];

#endif /* GUI_WIDGETS_WIDGET_FLAGS_H_ */
