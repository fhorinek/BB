/*
 * common.c
 *
 *  Created on: 17. 1. 2022
 *      Author: horinek
 */


#include "common.h"
#include <sys/time.h>
#include "etc/geo_calc.h"

gui_t gui;

struct timeval start;

uint32_t HAL_GetTick()
{
	static uint64_t start = 0;

	struct timeval tv;
	gettimeofday(&tv, NULL);

	uint64_t tick = tv.tv_usec / 1000;
	if (start == 0)
		start = tick;

	return tick - start;
}


void get_tmp_path(char * fname, uint32_t id)
{
    sprintf(fname, "%s/%08lX", PATH_TEMP_DIR, id);
}


uint32_t get_tmp_filename(char * fname)
{
    static uint32_t counter = 0;

    if (fname != NULL)
    	get_tmp_path(fname, counter);

    uint32_t tmp = counter;
    counter++;

    return tmp;
}

bool file_exists(char * file)
{
	FIL f;

	if (f_open(&f, file, FA_READ) == FR_OK)
	{
		f_close(&f);
		return true;
	}

	return false;
}
