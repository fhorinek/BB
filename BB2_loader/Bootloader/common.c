#include "common.h"

void GpioSetDirection(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint16_t direction, uint16_t pull)
{
	  GPIO_InitTypeDef GPIO_InitStruct = {0};

	  GPIO_InitStruct.Pin = GPIO_Pin;
	  GPIO_InitStruct.Mode = (direction == OUTPUT) ? GPIO_MODE_OUTPUT_PP : GPIO_MODE_INPUT;
	  GPIO_InitStruct.Pull = pull;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}


void gui_set_backlight(uint8_t val)
{
    if (val > 100)
        val = 100;

    __HAL_TIM_SET_COMPARE(&led_timmer, led_bclk, val);
}

void gui_set_torch(uint8_t val)
{
    if (val > 100)
        val = 100;

    __HAL_TIM_SET_COMPARE(&led_timmer, led_torch, val);
}
