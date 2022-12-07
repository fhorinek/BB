/*
 * tmalloc.h
 *
 *  Created on: Nov 5, 2022
 *      Author: horinek
 */

#ifndef ETC_TMALLOC_H_
#define ETC_TMALLOC_H_

#include "common.h"

#define MALLOC_TRACE

#ifdef MALLOC_TRACE

#define OVERFLOW_DETECTION

void tmalloc_init();

//macro to get filename and line number
#define tmalloc(size) tmalloc_real(size, __FILE_NAME__, __LINE__)
#define tfree(ptr) tfree_real(ptr, __FILE_NAME__, __LINE__)


//malloc functions with tracing
void * tmalloc_real(uint32_t requested_size, char * name, uint32_t lineno);
void tfree_real(void * ptr, char * name, uint32_t lineno);

//print traced memory
void tmalloc_print();
void tmalloc_summary_info();
void tmalloc_check();
void bsod_tmalloc_info();

void tmalloc_summary_info_unlocked();

#ifdef OVERFLOW_DETECTION
void tmalloc_check();
#else
#define tmalloc_check() INFO("Overflow detection disabled");
#endif

#else

#define tmalloc_init() INFO("Trace malloc disabled");

#define tmalloc(size)   malloc_check(size, __FILE_NAME__, __LINE__)
#define tfree(ptr)      free(ptr)


#define tmalloc_print() INFO("Trace malloc disabled");
#define tmalloc_summary_info() INFO("Trace malloc disabled");
#define tmalloc_check() INFO("Trace malloc disabled");
#define bsod_tmalloc_info()

#endif

#define FILE_NAME_SIZE  16
#define NUMBER_OF_FILES 64

extern char trace_filenames[NUMBER_OF_FILES][FILE_NAME_SIZE];
void trace_init();
uint16_t trace_find_filename_slot(char * name);


#endif /* ETC_TMALLOC_H_ */
