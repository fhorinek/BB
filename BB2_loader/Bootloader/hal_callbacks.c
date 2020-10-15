/*
 * hal_callbacks.c
 *
 *  Created on: Sep 17, 2020
 *      Author: horinek
 |
 *      This file is router for HAL callbacks
 */


#include "common.h"
#include "drivers/bq25895.h"
#include "drivers/tft_hx8352.h"

void HAL_SD_ErrorCallback(SD_HandleTypeDef *hsd)
{
	ERR("HAL_SD: %08X", hsd->ErrorCode);
	//Error_Handler();
}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{

  switch (GPIO_Pin)
  {
	  case(PWR_INT_Pin):
			bq25895_irq();
	  break;

	  case(DISP_TE_Pin):
			tft_irq_display_te();
	  break;

  }

}
