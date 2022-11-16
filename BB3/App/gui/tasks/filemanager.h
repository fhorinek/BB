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

void filemanager_refresh();

#define FM_CB_BACK          0xF0    //back button pressed in filemanager we are in the directory where filemanager was opened, If callback returns true, filemanager gets closed and return to previous gui_task_t
#define FM_CB_SELECT        0xF1    //file gets selected. If callback returns true, filemanager gets closed and return to previous gui_task_t
#define FM_CB_FOCUS_FILE    0xF2    //new file was focused, can be used to create file specific context menu, return is ignored
#define FM_CB_FOCUS_DIR     0xF3    //new folder was focused, can be used to create folder specific context menu, return is ignored
#define FM_CB_FILTER        0xF4    //called for every file during filemanager_open, if callback return false, the file will be hidden, need to use FM_FLAG_FILTER flag
#define FM_CB_CANCEL        0xF5    //cancel button pressed in filemanager, If callback returns true, filemanager will go up one directory level, or return to previous gui_task_t if in the directory where filemanager was opened
#define FM_CB_INIT          0xF6    //called befor filemanager_open listing, can be used to add help_set_base
#define FM_CB_APPEND        0xF7    //called after filemanager_open listing, can be used to add custom menu item at the end
//numerical callbacks event are for context menu callbacks 0-first ctx item, 1-second ctx item, ...


#define FM_FLAG_FILTER      0b00000001  //enable file filtering via FM_CB_FILTER callback
#define FM_FLAG_FOCUS       0b00000010  //enable callback for FM_CB_FOCUS_FILE and FM_CB_FOCUS_DIR
#define FM_FLAG_HIDE_FILE   0b00000100  //hide files
#define FM_FLAG_HIDE_DIR    0b00001000  //hide folders
#define FM_FLAG_SHOW_EXT    0b00010000  //do not hide file ext
#define FM_FLAG_SORT_NAME   0b00100000  //sort by name
#define FM_FLAG_SORT_DATE   0b01000000  //sort by date

#define FM_FILE_MAX_COUNT   100         // the maximum number of files to show

#endif /* GUI_FILEMANAGER_H_ */
