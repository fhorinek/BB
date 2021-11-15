/*
 * filemanager.h
 *
 *  Created on: Jul 13, 2021
 *      Author: horinek
 */

#ifndef GUI_FILEMANAGER_H_
#define GUI_FILEMANAGER_H_

#include "gui/gui.h"

typedef bool (* filemanager_cb_t)(uint8_t, char *);

DECLARE_TASK(filemanager);
void filemanager_open(char * path, uint8_t level, gui_task_t * back, uint8_t flags, filemanager_cb_t cb);
bool filemanager_back();

bool filemanager_get_filename_no_ext(char * dst, char * path);
bool filemanager_get_filename(char * dst, char * path);
bool filemanager_get_path(char * dst, char * path);

#define FM_CB_CANCEL        0xF0
#define FM_CB_SELECT        0xF1
#define FM_CB_FOCUS_FILE    0xF2
#define FM_CB_FOCUS_DIR     0xF3
#define FM_CB_FILTER        0xF4

#define FM_FLAG_FILTER      0b00000001
#define FM_FLAG_FOCUS       0b00000010
#define FM_FLAG_HIDE_FILE   0b00000100
#define FM_FLAG_HIDE_DIR    0b00001000

#endif /* GUI_FILEMANAGER_H_ */
