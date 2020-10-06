/*
 * hal_callbacks.c
 *
 *  Created on: Sep 17, 2020
 *      Author: horinek
 |
 *      This file is router for HAL callbacks
 */


#include "common.h"

void HAL_SD_ErrorCallback(SD_HandleTypeDef *hsd)
{
	ERR("HAL_SD: %08X", hsd->ErrorCode);
	//Error_Handler();
}
