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

static uint32_t led_timeout = 0;

#define LED_MUL		256
#define LED_WAIT	10

void led_init()
{
	//init PWM
    HAL_TIM_Base_Start_IT(led_timer);
    HAL_TIM_PWM_Start(led_timer, led_torch);

    HAL_TIM_Base_Start_IT(disp_timer);
    HAL_TIM_PWM_Start(disp_timer, led_bclk);
}

void led_set_backlight(uint8_t val)
{
	if (val > 100)
		val = 100;

	val += LED_WAIT;
	led_bckl_target = val * LED_MUL;
}

void led_set_backlight_timeout(uint16_t timeout)
{
	if (timeout != 0)
		led_timeout = HAL_GetTick() + timeout;
	else
		led_timeout = 0;
}

void led_set_torch(uint8_t val)
{
	if (val > 100)
		val = 100;

	led_torch_target = val * LED_MUL;
}

void disp_period_irq()
{
	if (led_bckl_actual != led_bckl_target)
	{
		if (led_bckl_actual > led_bckl_target)
			led_bckl_actual--;
		else
			led_bckl_actual++;

		uint16_t tmp = led_bckl_actual / LED_MUL;
		if (tmp < LED_WAIT)
			tmp = 0;
		else
			tmp -= LED_WAIT;

		__HAL_TIM_SET_COMPARE(disp_timer, led_bclk, tmp);
	}

	if (led_timeout != 0 && led_timeout < HAL_GetTick())
	{
		led_timeout = 0;
		led_bckl_target = 0;
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

		__HAL_TIM_SET_COMPARE(led_timer, led_torch, led_torch_actual / LED_MUL);
	}
}

void led_dim()
{
	led_set_backlight(0);
	led_set_torch(0);

	while(led_torch_actual != led_torch_target
			|| led_bckl_actual != led_bckl_target)
	{
		HAL_Delay(1);
	}
}
