/*
 * filemanager.h
 *
 *  Created on: Jul 13, 2021
 *      Author: horinek
 */

#ifndef GUI_FILEMANAGER_H_
#define GUI_FILEMANAGER_H_

#include "gui/gui.h"

typedef void (* filemanager_cb_t)(char *);

DECLARE_TASK(filemanager);
void filemanager_open(char * path, uint8_t level, gui_task_t * back, filemanager_cb_t cb);

#endif /* GUI_FILEMANAGER_H_ */
