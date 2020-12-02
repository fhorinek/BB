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

#ifndef DBG_LEVEL
#define DBG_LEVEL DBG_INFO
#endif

#if DBG_LEVEL <= DBG_DEBUG
#define DBG(...)	debug_send(DBG_DEBUG, __VA_ARGS__)
#else
#define DBG(...)
#endif

#if DBG_LEVEL <= DBG_INFO
#define INFO(...)	debug_send(DBG_INFO, __VA_ARGS__)
#else
#define INFO(...)
#endif

#if DBG_LEVEL <= DBG_WARN
#define WARN(...)	debug_send(DBG_WARN, __VA_ARGS__)
#else
#define WARN(...)
#endif

#if DBG_LEVEL <= DBG_ERROR
#define ERR(...)	debug_send(DBG_DEBUG, __VA_ARGS__)
#else
#define ERR(...)
#endif

#define DUMP(data, len)	debug_dump(data, len)

#define ASSERT(cond)	\
	do {	\
		if (!(cond))	\
			ERR("Assertion failed %s:%u", __FILENAME__, __LINE__); \
	} while(0);

#ifdef __cplusplus
extern "C" {
#endif

void debug_send(uint8_t type, const char *format, ...);
void debug_dump(uint8_t * data, uint16_t len);
void debug_uart_done();

typedef struct
{
	char * sender;
	uint8_t type;
	char * message;
} debug_msg_t;

#ifdef __cplusplus
}
#endif

#endif /* DEBUG_H_ */
