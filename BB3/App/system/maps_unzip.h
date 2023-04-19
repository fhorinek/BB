/*
 * maps_unzip.h
 *
 * Unzip all ZIP files inside the maps directory and delete ZIP afterwards.
 *
 *  Created on: Feb 27, 2023
 *      Author: tilmann@bubecks.de
 */

#ifndef __MAPS_UNZIP_H
#define __MAPS_UNZIP_H

void maps_unzip_start_task();
bool unzip_zipfile(char *target_dir, char *zip_file_path, bool gui);

#endif

