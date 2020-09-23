/*
 * fc.cc
 *
 *  Created on: May 6, 2020
 *      Author: horinek
 */


#include "fc.h"

fc_t fc;

void fc_init()
{
	INFO("Flight computer init");
	//Release the semaphore
	osSemaphoreRelease(fc_global_lockHandle);

}
