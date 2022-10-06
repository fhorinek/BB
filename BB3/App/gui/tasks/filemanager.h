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

uint8_t filemanager_get_current_level();

bool filemanager_get_filename_no_ext(char * dst, char * path);
bool filemanager_get_filename(char * dst, char * path);
bool filemanager_get_path(char * dst, char * path);

#define FM_CB_BACK          0xF0
#define FM_CB_SELECT        0xF1   // file gets selected. If callback returns true, filemanager gets closed and return to previous gui_task_t
#define FM_CB_FOCUS_FILE    0xF2
#define FM_CB_FOCUS_DIR     0xF3
#define FM_CB_FILTER        0xF4
#define FM_CB_CANCEL        0xF5
#define FM_CB_APPEND        0xF6

#define FM_FLAG_FILTER      0b00000001
#define FM_FLAG_FOCUS       0b00000010
#define FM_FLAG_HIDE_FILE   0b00000100
#define FM_FLAG_HIDE_DIR    0b00001000
#define FM_FLAG_SHOW_EXT    0b00010000
#define FM_FLAG_SORT_NAME   0b00100000
#define FM_FLAG_SORT_DATE   0b01000000

#define FM_FILE_MAX_COUNT   100         // the maximum number of files to show

#endif /* GUI_FILEMANAGER_H_ */
