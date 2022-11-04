/*
 * bosd.h
 *
 *  Created on: 7. 6. 2021
 *      Author: horinek
 */

#ifndef SYSTEM_BSOD_H_
#define SYSTEM_BSOD_H_

#include "common.h"
#include "lib/CrashCatcher/CrashCatcher.h"

void bsod_msg(const char *format, ...);

void bsod_crash_start(const Crash_Object * info);
void bsod_crash_dumped();
void bsod_crash_fail();

extern char * bsod_msg_ptr;
#define BSOD_MSG_SIZE   256

extern char *bsod_strato_crashed_msg;
#define BSOD_STRATO_CRASHED_MESSAGE "** Strato Crashed **"

#endif /* SYSTEM_BSOD_H_ */
