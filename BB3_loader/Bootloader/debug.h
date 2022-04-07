/*
 * debug.h
 *
 *  Created on: Apr 17, 2020
 *      Author: horinek
 */

#ifndef DEBUG_H_
#define DEBUG_H_

#include "common.h"



#define DBG_DEBUG	0
#define DBG_INFO	1
#define DBG_WARN	2
#define DBG_ERROR	3

#define DBG(...)	debug_send(DBG_DEBUG, __VA_ARGS__)
#define INFO(...)	debug_send(DBG_INFO, __VA_ARGS__)
#define WARN(...)	debug_send(DBG_WARN, __VA_ARGS__)
#define ERR(...)	debug_send(DBG_ERROR, __VA_ARGS__)

#define DUMP(data, len) debug_dump(data, len)

#undef __FILENAME__
#define __FILENAME__ __FILE_NAME__

#define ASSERT(cond)	\
	do {	\
		if (!(cond))	\
			ERR("Assertion failed %s:%u", __FILENAME__, __LINE__); \
	} while(0);

#define ASSERT_MSG(cond, ...)    \
    do {    \
        if (!(cond))  {  \
            ERR("Assertion failed %s:%u", __FILENAME__, __LINE__); \
            ERR(__VA_ARGS__); \
            while(1); }\
    } while(0);


void debug_enable();
void debug_send(uint8_t type, const char *format, ...);

#endif /* DEBUG_H_ */
