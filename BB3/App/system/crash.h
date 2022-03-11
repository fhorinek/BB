/*
 * crash.h
 *
 *  Created on: Mar 9, 2022
 *      Author: horinek
 */

#ifndef SYSTEM_CRASH_H_
#define SYSTEM_CRASH_H_

#include "common.h"
#include "lib/CrashCatcher/CrashCatcher.h"

void CrashCatcher_DumpStart(const Crash_Object * info);
void CrashCatcher_DumpMemory(const void* pvMemory, CrashCatcherElementSizes elementSize, size_t elementCount);
CrashCatcherReturnCodes CrashCatcher_DumpEnd(void);

#endif /* SYSTEM_CRASH_H_ */
