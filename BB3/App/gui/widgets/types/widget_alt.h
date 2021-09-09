/*
 * widget_alt.h
 *
 *  Created on: 8. 9. 2021
 *      Author: horinek
 */

#ifndef GUI_WIDGETS_TYPES_WIDGET_ALT_H_
#define GUI_WIDGETS_TYPES_WIDGET_ALT_H_

void Alt_init(void * local_ptr, lv_obj_t * base, widget_slot_t * slot, char * title);
void Alt_edit(void * local_ptr, widget_slot_t * slot, uint8_t action, char * title, float * alt, cfg_entry_t * qnh);
void Alt_update(void * local_ptr, widget_slot_t * slot, float alt, int32_t qnh_val);
void Alt_stop(void * local_ptr, widget_slot_t * slot);

#endif /* GUI_WIDGETS_TYPES_WIDGET_ALT_H_ */
