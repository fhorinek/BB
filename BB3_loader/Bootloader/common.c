#include "common.h"

bool button_pressed(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    return HAL_GPIO_ReadPin(GPIOx, GPIO_Pin) == LOW;
}

void button_wait(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    while(HAL_GPIO_ReadPin(GPIOx, GPIO_Pin) == LOW);
}

void button_confirm(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    while(HAL_GPIO_ReadPin(GPIOx, GPIO_Pin) == LOW);
    while(HAL_GPIO_ReadPin(GPIOx, GPIO_Pin) != LOW);
}

#define HOLD_TIME  750
#define HOLD_NONE   0xFF

bool button_hold_2(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint32_t timeout)
{
    static GPIO_TypeDef * use_port = 0;
    static uint16_t use_pin = HOLD_NONE;
    static uint32_t hold_start;

    if (HAL_GPIO_ReadPin(GPIOx, GPIO_Pin) == LOW)
    {
        if (use_port == GPIOx && use_pin == GPIO_Pin)
        {
            if (HAL_GetTick() - hold_start > timeout && hold_start != 0)
            {
                hold_start = 0;
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            hold_start = HAL_GetTick();
            use_port = GPIOx;
            use_pin = GPIO_Pin;

            return false;
        }
    }

    if (use_port == GPIOx && use_pin == GPIO_Pin)
    {
        use_pin = HOLD_NONE;
    }

    return false;
}
bool button_hold(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    return button_hold_2(GPIOx, GPIO_Pin, HOLD_TIME);
}

bool file_exists(char * path)
{
    FILINFO fno;
    return (f_stat(path, &fno) == FR_OK);
}

void GpioSetDirection(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint16_t direction, uint16_t pull)
{
	  GPIO_InitTypeDef GPIO_InitStruct = {0};

	  GPIO_InitStruct.Pin = GPIO_Pin;
	  GPIO_InitStruct.Mode = (direction == OUTPUT) ? GPIO_MODE_OUTPUT_PP : GPIO_MODE_INPUT;
	  GPIO_InitStruct.Pull = pull;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}



int16_t complement2_16bit(uint16_t in)
{
#define MASK 0b0111111111111111

	if (in > MASK)
		return (in & MASK) - (MASK + 1);

	return in;
}
