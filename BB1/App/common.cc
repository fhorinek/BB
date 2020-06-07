#include "common.h"

#include "drivers/tft_hx8352.h"

void GpioSetDirection(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint16_t direction, uint16_t pull)
{
	  GPIO_InitTypeDef GPIO_InitStruct = {0};

	  GPIO_InitStruct.Pin = GPIO_Pin;
	  GPIO_InitStruct.Mode = (direction == OUTPUT) ? GPIO_MODE_OUTPUT_PP : GPIO_MODE_INPUT;
	  GPIO_InitStruct.Pull = pull;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == DISPLAY_TE_Pin)
	{
		tft_irq_display_te();
	}
}

//extern "C" void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart == &debug_uart)
		debug_uart_done();
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
//	while(1);
}

uint8_t hex_to_num(uint8_t c)
{
	if (c >= 'A')
		return c - 'A' + 10;
	else
		return c - '0';
}

uint8_t atoi_hex8(char * buffer)
{
	return hex_to_num(buffer[0]) << 4 | hex_to_num(buffer[1]);
}

uint32_t atoi_hex32(char * buffer)
{
	uint8_t c;
	uint32_t res = 0;

	for (uint8_t i = 0; i < 8; i++)
	{
		c = buffer[i];
		if (c == ',' || c == 0)
			break;

		res <<= 4;
		res |= hex_to_num(c);
	}

	return res;
}

bool start_with(char * s1, const char * s2)
{
	for (uint8_t i = 0; i < strlen(s2); i++)
	{
		if (s1[i] != s2[i])
			return false;
	}

	return true;
}

char * find_comma(char * str)
{
	return index(str, ',') + 1;
}

uint32_t pow_ten(uint8_t pow)
{
	uint32_t ret = 1;
	uint8_t i;

	for (i = 0; i < pow; i++)
		ret *= 10;

	return ret;
}

uint16_t atoi_c(char * str)
{
	uint16_t tmp = 0;
	uint8_t i = 0;

	while(ISDIGIT(str[i]))
	{
		tmp *= 10;
		tmp += str[i] - '0';
		i++;
	}

	return tmp;
}

uint32_t atoi_n(char * str, uint8_t n)
{
	uint32_t tmp = 0;
    uint8_t i;

	for (i=0; i < n; i++)
	{
		if (str[i] == ',')
			return tmp;
		if (str[i] == '.')
		{
			n++;
			continue;
		}

		tmp += (uint32_t)(str[i] - '0') * pow_ten(n - i - 1);
	}

	return tmp;
}

float atoi_f(char * str)
{
	float tmp = 0;
	uint8_t dot = 0;
	uint8_t i = 0;

	if (str[0] == '-')
		i++;

	while(ISDIGIT(str[i]) || str[i] == '.')
	{
		if (str[i] == '.')
		{
			dot = i;
			i++;
		}

		if (dot == 0)
		{
			tmp *= 10;
			tmp += str[i] - '0';
		}
		else
		{
			tmp += (str[i] - '0') / pow(10, i - dot);
		}

		i++;
	}

	if (str[0] == '-')
		tmp *= -1;

	return tmp;
}

int8_t complement7(uint8_t in)
{
#define MASK 0b00111111

	if (in > MASK)
		return (in & MASK) - (MASK + 1);

	return in;
}


