/*
 * hx8352.c
 *
 *  Created on: Apr 16, 2020
 *      Author: horinek
 *
 *  Frame buffer resides in main memory
 *  Display GRAM is mapped via FMC
 *  Data transfer starts during blanking time (when TE goes high) and buffer is mofified
 *  Transfer is done via DMA, it must ends before TE goes low
 *  blanking time can be modified via reg 0x2b
 *
 */

#include "../drivers/tft_hx8352.h"

#include "../debug.h"

uint16_t * tft_register = (uint16_t *)0xC0000000;
uint16_t * tft_ram = (uint16_t *)0xC0020000;

extern DMA_HandleTypeDef hdma_memtomem_dma2_stream0;

uint16_t tft_buffer[TFT_BUFFER_SIZE];
bool tft_buffer_ready = false;
bool tft_dma_done = true;

uint16_t tft_win_x1, tft_win_x2;
uint16_t tft_win_y1, tft_win_y2;

bool tft_buffer_copy()
{
	if (HAL_DMA_GetState(&hdma_memtomem_dma2_stream0) != HAL_DMA_STATE_READY)
	{
		return false;
	}

	tft_set_window(tft_win_x1, tft_win_y1, tft_win_x2, tft_win_y2);
	tft_set_cursor(tft_win_x1, tft_win_y1);

	//DMA transfer length is in number of transactions!!!
	//chunks are set to 32b, because DMA transfer length is only 16-bit wide
	//DMA limit is therefore 4x65535 = 262,140 (we need 192,000)
	uint32_t len;
	len = (tft_win_x2 - tft_win_x1 + 1) * (tft_win_y2 - tft_win_y1 + 1);
	len /= 2;

	*tft_register = 0x22;
	HAL_DMA_Start_IT(&hdma_memtomem_dma2_stream0, (uint32_t)tft_buffer, (uint32_t)tft_ram, len);
	return true;
}

void tft_irq_display_te()
{
	if (tft_buffer_ready)
	{
		tft_buffer_ready = false;
		tft_buffer_copy();
	}
}

void tft_irq_dma_done(DMA_HandleTypeDef *DmaHandle)
{
	tft_dma_done = true;
}

void tft_wait_for_buffer()
{
	while(!tft_dma_done);
}

void tft_refresh_buffer(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	tft_wait_for_buffer();

	tft_win_x1 = x1;
	tft_win_y1 = y1;

	tft_win_x2 = x2;
	tft_win_y2 = y2;

	tft_buffer_ready = true;

	tft_dma_done = false;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == DISPLAY_TE_Pin)
	{
		tft_irq_display_te();
	}
}


void tft_init()
{
	//init vars
	tft_buffer_ready = false;

	//set pins
	GpioSetDirection(DISPLAY_TE, INPUT);

	//set fmc (in main via MX)

	//relocate FMC memory outside cache region
	SYSCFG->MEMRMP |= SYSCFG_MEMRMP_SWP_FMC_0;

	//DMA callback
	hdma_memtomem_dma2_stream0.XferCpltCallback = tft_irq_dma_done;
}

void tft_reset()
{
	HAL_GPIO_WritePin(DISPLAY_RESET, HIGH);
	HAL_Delay(50);
    HAL_GPIO_WritePin(DISPLAY_RESET, LOW);
    HAL_Delay(100);
    HAL_GPIO_WritePin(DISPLAY_RESET, HIGH);
    HAL_Delay(100);
}


void tft_write_register(uint16_t command, uint16_t data)
{
	*tft_register = command;
	tft_ram[0] = data;
}

uint16_t tft_read_register(uint16_t command)
{
	*tft_register = command;
	uint16_t mem[2];
	//dummy read
	mem[0] = tft_ram[0];
	//real read
	mem[1] = tft_ram[1];

	return mem[1];
}

void tft_color_fill(uint16_t color)
{
 	for (uint32_t i = 0; i < TFT_BUFFER_SIZE; i++)
 		tft_buffer[i] = color;
}

void tft_test_pattern()
{
	uint8_t r, g, b;

 	for (uint32_t i = 0; i < TFT_BUFFER_SIZE; i++)
 	{
 		uint16_t x = i % 240;
 		uint16_t y = i / 240;

 		r = g = b = 0;

 		if (y < 256)
 		{
 			//color test
			if (x < 10)			r= 255;
			else if (x < 80)	r = y;
			else if (x < 90)	g = 255;
			else if (x < 160)	g = y;
			else if (x < 170)	b = 255;
			else				b = y;
 		}
 		else
 		{
 			//chessboard pattern
 			bool q = (y % 20 < 10);
 			bool w = (x % 20 >= 10);
 			if (q != w)
 				r = g = b = 255;

 		}

 		//convert 24bit to 16bit (565)
 		r >>= 3;
 		g >>= 2;
 		b >>= 3;

 		tft_buffer[i] = (r << 11) | (g << 5) | b;

 	}
}


#define TFTLCD_DELAY 0x7F

static void tft_init_table(const uint8_t *table, uint16_t size)
{
    for (uint8_t i = 0; i < size; i += 2)
	{
        uint8_t cmd = table[i];
        uint8_t data = table[i + 1];
        if (cmd == TFTLCD_DELAY)
        	HAL_Delay(data);
        else
        {
        	tft_write_register(cmd, data);
//        	debug("writing %04X %04X -> %04X\n", cmd, data, tft_read_register(cmd));
        }
    }
}

void tft_set_rotation(uint8_t r)
{
    uint8_t val;

    switch (r) {
    case 0:                    //PORTRAIT:
        val = 0b11000000;             //MY=0, MX=1, MV=0, ML=0, BGR=1
        break;
    case 1:                    //LANDSCAPE: 90 degrees
        val = 0b10100000;             //MY=0, MX=0, MV=1, ML=0, BGR=1
        break;
    case 2:                    //PORTRAIT_REV: 180 degrees
        val = 0b00000000;             //MY=1, MX=0, MV=0, ML=1, BGR=1
        break;
    case 3:                    //LANDSCAPE_REV: 270 degrees
        val = 0b00100010;             //MY=1, MX=1, MV=1, ML=1, BGR=1
        break;
    }

	tft_write_register(0x16, val | 0x08);
}

void tft_init_display()
{
	tft_reset();

	static const uint8_t HX8352B_init[] = {
		// Register setting for EQ setting
		0xe5, 0x10,      //
		0xe7, 0x10,      //
		0xe8, 0x48,      //
		0xec, 0x09,      //
		0xed, 0x6c,      //

		// Power on Setting
		0x23, 0x6F,      //VMF
		0x24, 0x57,      //VMH
		0x25, 0x71,      //VML
		0xE2, 0x18,      //
		0x1B, 0x15,      //VRH
		0x01, 0x00,      //
		0x1C, 0x03,      //AP=3

		// Power on sequence
		0x19, 0x01,      //OSCEN=1
		TFTLCD_DELAY, 5,
		0x1F, 0x8C,      //GASEN=1, DK=1, XDK=1
		0x1F, 0x84,      //GASEN=1, XDK=1
		TFTLCD_DELAY, 10,
		0x1F, 0x94,      //GASEN=1, PON=1, XDK=1
		TFTLCD_DELAY, 10,
		0x1F, 0xD4,      //GASEN=1, VCOMG=1, PON=1, XDK=1
		TFTLCD_DELAY, 5,

		0x28, 0x20,      //GON=1
		TFTLCD_DELAY, 40,
		0x28, 0x38,      //GON=1, DTE=1, D=2
		TFTLCD_DELAY, 40,

		0x17, 0x05,		//COLMODE
		0x60, 0x08,		//TE
		//0x2b, 220,		//Blanking time (transfer time) Tearing control
	};

	static const uint8_t HX8352B_disp_on[] = {
		0x28, 0x3C,      //GON=1, DTE=1, D=3
	};

	tft_init_table(HX8352B_init, sizeof(HX8352B_init));

	tft_set_rotation(0);             //PORTRAIT
	tft_color_fill(0xFFFF);
	tft_refresh_buffer(0, 0, 239, 399);
	tft_wait_for_buffer();

	tft_init_table(HX8352B_disp_on, sizeof(HX8352B_disp_on));
}

void tft_set_window(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	tft_write_register(0x02, x1 >> 8);
	tft_write_register(0x03, x1 & 0xFF);

	tft_write_register(0x04, x2 >> 8);
	tft_write_register(0x05, x2 & 0xFF);

	tft_write_register(0x06, y1 >> 8);
	tft_write_register(0x07, y1 & 0xFF);

	tft_write_register(0x08, y2 >> 8);
	tft_write_register(0x09, y2 & 0xFF);
}

void tft_set_cursor(uint16_t x, uint16_t y)
{
	tft_write_register(0x80, x >> 8);
	tft_write_register(0x81, x);

	tft_write_register(0x82, y >> 8);
	tft_write_register(0x83, y);

}

