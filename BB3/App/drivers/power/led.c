/*
 * led.c
 *
 *  Created on: Oct 27, 2020
 *      Author: horinek
 */

#include "led.h"

static uint16_t led_bckl_target = 0;
static uint16_t led_bckl_actual = 0;
static uint16_t led_torch_target = 0;
static uint16_t led_torch_actual = 0;

void led_retrive_bckl_compare()
{
	led_bckl_actual = TIM2->CCR4 * 128;
	led_bckl_target = led_bckl_actual;
}

void led_init()
{
	//init PWM
    HAL_TIM_Base_Start_IT(led_timer);
    HAL_TIM_PWM_Start(led_timer, led_torch);

    led_retrive_bckl_compare();

    MX_TIM2_Init();

    HAL_TIM_Base_Start_IT(disp_timer);
    HAL_TIM_PWM_Start(disp_timer, led_bclk);
    __HAL_TIM_SET_COMPARE(disp_timer, led_bclk, led_bckl_actual / 128);
}

void led_set_backlight(uint8_t val)
{
	if (val > 100)
		val = 100;

	led_bckl_target = val * 128;
}

void led_set_torch(uint8_t val)
{
	if (val > 100)
		val = 100;

	led_torch_target = val * 128;
}

void disp_period_irq()
{
	if (led_bckl_actual != led_bckl_target)
	{
		if (led_bckl_actual > led_bckl_target)
			led_bckl_actual--;
		else
			led_bckl_actual++;

		__HAL_TIM_SET_COMPARE(disp_timer, led_bclk, led_bckl_actual / 128);
	}
}

void led_period_irq()
	{
	if (led_torch_actual != led_torch_target)
	{
		if (led_torch_actual > led_torch_target)
			led_torch_actual--;
		else
			led_torch_actual++;

		__HAL_TIM_SET_COMPARE(led_timer, led_torch, led_torch_actual / 128);
	}
}
