#include "common.h"

bool development_mode = false;

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
	//delay is in 10ms, 2000 == 20s
	uint16_t delay = 2000;

	//wait for button to be released
    while(HAL_GPIO_ReadPin(GPIOx, GPIO_Pin) == LOW)
    {
    	delay--;
    	if (delay == 0)
    		return;
    	HAL_Delay(10);
    }
    //wait for button to be pressed
    while(HAL_GPIO_ReadPin(GPIOx, GPIO_Pin) != LOW)
    {
    	delay--;
    	if (delay == 0)
    		return;
    	HAL_Delay(10);
    }
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
    struct lfs_info info;
    return (lfs_stat(&lfs, path, &info) == LFS_ERR_OK);
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

uint8_t calc_crc(uint8_t crc, uint8_t key, uint8_t data)
{
    uint8_t i;

    for (i = 0; i < 8; i++)
    {
        if ((data & 0x01) ^ (crc & 0x01))
        {
            crc = crc >> 1;
            crc = crc ^ key;
        }
        else
            crc = crc >> 1;
        data = data >> 1;
    }

    return crc;
}

int16_t complement2_16bit(uint16_t in)
{
#define MASK 0b0111111111111111

	if (in > MASK)
		return (in & MASK) - (MASK + 1);

	return in;
}

void clear_dir(char * path)
{
    char path_buff[512];
    strcpy(path_buff, path);
    //TODO implement this

}

