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
#include "drivers/tft/tft.h"
#include "drivers/led.h"



void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{

  switch (GPIO_Pin)
  {
	  case(BQ_INT_Pin):
			bq25895_irq();
	  break;

	  case(DISP_TE_Pin):
			tft_irq_display_te();
	  break;

  }

}

void HAL_TIM_PeriodElapsedCallback2(TIM_HandleTypeDef *htim)
{
    if (htim == disp_timer)
    {
        disp_period_irq();
    }

    if (htim == led_timer)
    {
        led_period_irq();
    }
}
