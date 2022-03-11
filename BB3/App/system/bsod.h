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

void bsod_show(context_frame_t * frame);
void bsod_msg(const char *format, ...);

void bsod_crash_start(const Crash_Object * info);
void bsod_crash_dumped();
void bsod_crash_fail();

#endif /* SYSTEM_BSOD_H_ */
