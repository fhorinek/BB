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

void led_init()
{
	//init PWM
	HAL_TIM_Base_Start_IT(&led_timmer);
	HAL_TIM_PWM_Start(&led_timmer, led_bclk);
	HAL_TIM_PWM_Start(&led_timmer, led_torch);
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

void led_period_irq()
{
	if (led_bckl_actual != led_bckl_target)
	{
		if (led_bckl_actual > led_bckl_target)
			led_bckl_actual--;
		else
			led_bckl_actual++;

		uint16_t tmp = led_bckl_actual / 128;

		__HAL_TIM_SET_COMPARE(&led_timmer, led_bclk, tmp);
	}

	if (led_torch_actual != led_torch_target)
	{
		if (led_torch_actual > led_torch_target)
			led_torch_actual--;
		else
			led_torch_actual++;

		__HAL_TIM_SET_COMPARE(&led_timmer, led_torch, led_torch_actual / 128);
	}
}
