/*
 * fake_fatfs.c
 *
 *  Created on: 17. 1. 2022
 *      Author: horinek
 */

#include "fake_fatfs.h"

FRESULT f_open(FIL * fp, char * path, uint8_t mode)
{
	char filename[512];
	sprintf(filename, "%s", path);

	char * smode = "?";

	if (mode == FA_READ)
		smode = "rb";
	if (mode == FA_WRITE || mode == (FA_WRITE | FA_CREATE_NEW) || mode == (FA_WRITE | FA_CREATE_ALWAYS))
		smode = "wb";
	if (mode == (FA_OPEN_ALWAYS | FA_WRITE | FA_READ))
		smode = "rwb";

	fp = fopen(filename, smode);

	return (fp != NULL) ? FR_OK : FR_ERROR;
}


FRESULT f_read(FIL * fp, void * buf, uint32_t r, UINT * rb)
{
	*rb = fread(buf, r, 1, fp);

	return FR_OK;
}

FRESULT f_write(FIL * fp, void * buf, uint32_t w, UINT * wb)
{
	*wb = fwrite(buf, w, 1, fp);

	return FR_OK;
}

FRESULT f_close(FIL * fp)
{
	if (fp->_mode != 0)
		fclose(fp);
	return FR_OK;
}

uint32_t f_size(FIL * fp)
{
	uint32_t pos = ftell(fp);
	fseek(fp, 0L, SEEK_END);
	uint32_t sz = ftell(fp);
	fseek(fp, pos, SEEK_SET);

	return sz;
}

FRESULT f_lseek(FIL * fp, uint32_t pos)
{
	fseek(fp, pos, SEEK_SET);

	return FR_OK;
}

bool f_eof(FIL * fp)
{
	return feof(fp);
}

uint32_t f_tell(FIL * fp)
{
	return f_tell(fp);
}


