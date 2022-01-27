/*
 * fake_fatfs.h
 *
 *  Created on: 17. 1. 2022
 *      Author: horinek
 */

#ifndef MAP_ETC_FAKE_FATFS_H_
#define MAP_ETC_FAKE_FATFS_H_

#include "../common.h"

#include <sys/stat.h>

#define FIL 	FILE *
#define FRESULT uint8_t
#define UINT	uint32_t

#define FR_OK		0
#define FR_ERROR	1

#define FA_READ				0b00000001
#define FA_WRITE			0b00000010
#define FA_CREATE_ALWAYS	0b00000100
#define FA_CREATE_NEW		0b00001000
#define	FA_OPEN_APPEND		0b00010000
#define FA_OPEN_ALWAYS		0b00100000

FRESULT f_open(FIL * fp, char * path, uint8_t mode);
FRESULT f_read(FIL * fp, void * buf, uint32_t r, UINT * rb);
FRESULT f_write(FIL * fp, void * buf, uint32_t w, UINT * wb);
FRESULT f_close(FIL * fp);
uint32_t f_size(FIL * fp);
FRESULT f_lseek(FIL * fp, uint32_t pos);
bool f_eof(FIL * fp);
uint32_t f_tell(FIL * fp);
#define f_gets(str, size, fp) fgets(str, size, *fp)
#define f_unlink(path) remove(path)
#define f_rename(old, new) rename(old, new)
#define f_mkdir(path)	mkdir(path, 0777);

#endif /* MAP_ETC_FAKE_FATFS_H_ */
