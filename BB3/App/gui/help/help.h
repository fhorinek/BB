/*
 * help.h
 *
 *  Created on: 18. 10. 2022
 *      Author: horinek
 */

#ifndef GUI_HELP_HELP_H_
#define GUI_HELP_HELP_H_

#include "common.h"

#define HELP_ID_LEN 32

void help_set_base(char * lang_id);
void help_unset();

bool help_avalible(char * id);

bool help_show_icon_if_avalible(char * id);
bool help_show_icon_if_avalible_from_title(char * title);

void help_show(char * id);
void help_show_from_title(char * title);


void help_icon_show();
void help_icon_hide();

#endif /* GUI_HELP_HELP_H_ */
