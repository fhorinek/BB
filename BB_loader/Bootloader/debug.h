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

#define ASSERT(cond)	\
	do {	\
		if (!(cond))	\
			ERR("Assertion failed %s:%u", __FILENAME__, __LINE__); \
	} while(0);

#ifdef __cplusplus
extern "C" {
#endif

void debug_send(uint8_t type, const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif /* DEBUG_H_ */
