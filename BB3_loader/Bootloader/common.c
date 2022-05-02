#include "common.h"

bool development_mode = false;

//usable without IRQ
uint32_t HAL_GetTick(void)
{
  return sys_timer->Instance->CNT / 8;
}


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
    int32_t f = red_open(path, RED_O_RDONLY);
    if (f > 0)
    {
        red_close(f);
        return true;
    }
    return false;
}

uint64_t file_size(int32_t file)
{
    REDSTAT stat;
    red_fstat(file, &stat);

    return stat.st_size;
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

//not using memcpy to prevent unaligned access
void str_join(char * dst, uint8_t cnt, ...)
{
    va_list vl;
    va_start(vl, cnt);

    //move to end
    char * ptr = dst + strlen(dst);

    for (uint8_t i=0; i < cnt; i++)
    {
        char * val = va_arg(vl, char *);
        while(*val != 0)
        {
            *ptr = *val;
            ptr++;
            val++;
        }
    }
    va_end(vl);
    *ptr = 0;
}


void remove_dir_rec(char * path, bool remove)
{
    REDDIR * dir = red_opendir(path);
    if (dir != NULL)
    {
        while (true)
        {
            REDDIRENT * entry = red_readdir(dir);
            if (entry == NULL)
                break;

            char work_path[PATH_LEN] = {0};
            str_join(work_path, 3, path, "/", entry->d_name);

            if (RED_S_ISDIR(entry->d_stat.st_mode))
            {
                remove_dir_rec(work_path, true);
            }
            else
            {
                red_unlink(work_path);
            }
        }

        red_closedir(dir);

        if (remove)
            red_unlink(path);
    }
}

void clear_dir(char * path)
{
    remove_dir_rec(path, false);
}

