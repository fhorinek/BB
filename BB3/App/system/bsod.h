/*
 * bosd.h
 *
 *  Created on: 7. 6. 2021
 *      Author: horinek
 */

#ifndef SYSTEM_BSOD_H_
#define SYSTEM_BSOD_H_

#include "common.h"

void bsod_show(context_frame_t * frame);
void bsod_msg(const char *format, ...);

#endif /* SYSTEM_BSOD_H_ */
