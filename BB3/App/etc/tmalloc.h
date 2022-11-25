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

void tmalloc_init();

//macro to get filename and line number
#define tmalloc(size) tmalloc_real(size, __FILE_NAME__, __LINE__)
#define tfree(ptr) tfree_real(ptr, __FILE_NAME__, __LINE__)

#define tmalloc_sum(size, slot) tmalloc_real(size, slot, 0xFFFFFFFF)
#define tfree_sum(ptr, slot, size) tfree_real(ptr, slot, -size)


//malloc functions with tracing
void * tmalloc_real(uint32_t requested_size, char * name, uint32_t lineno);
void tfree_real(void * ptr, char * name, int32_t lineno);

//print traced memory
void tmalloc_print();
void tmalloc_summary_info();

//tag slots,
//slot will store the tag number during tmalloc
//useful when debugging iterations
void tmalloc_tag(uint16_t tag);
void tmalloc_tag_inc();

#else

#define tmalloc_init() INFO("Trace malloc disabled");

#define tmalloc(size)   malloc_check(size, __FILE_NAME__, __LINE__)
#define tfree(ptr)      free(ptr)

#define tmalloc_2(size, slot) malloc(size)
#define tfree_2(ptr, slot, size) free(ptr)

#define tmalloc_print() INFO("Trace malloc disabled");
#define tmalloc_summary_info() INFO("Trace malloc disabled");

#define tmalloc_tag(tag)
#define tmalloc_tag_inc()

#endif

#endif /* ETC_TMALLOC_H_ */
