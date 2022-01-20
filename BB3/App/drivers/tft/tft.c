/*
 * tft.c
 *
 *  Created on: 13. 4. 2021
 *      Author: horinek
 *
 *  Frame buffer resides in main memory
 *  Display GRAM is mapped via FMC
 *  Data transfer starts during blanking time (when TE goes high) and buffer is mofified
 *  Transfer is done via DMA, it must ends before TE goes low
 *  blanking time can be modified via reg 0x2b
 *
 */


#include "tft.h"

#include "hx8352.h"
#include "ili9327.h"

//Note: the NOR/PSRAM and SDRAM banks need to be swapped because of the weird caching on the default location
// do it in cube!
uint16_t * tft_register = (uint16_t *)0xC0000000;
uint16_t * tft_ram = (uint16_t *)0xC0020000;

uint16_t tft_buffer[TFT_BUFFER_SIZE];
uint16_t * tft_buffer_1 = tft_buffer;
uint16_t * tft_buffer_2 = tft_buffer + TFT_BUFFER_SIZE / 2;

uint16_t * tft_buffer_ptr;

static bool tft_buffer_ready = false;
static volatile bool tft_dma_done = false;

static uint16_t tft_win_x1, tft_win_x2;
static uint16_t tft_win_y1, tft_win_y2;

uint8_t tft_controller_type = 0xFF;

bool tft_normal_mode = true;

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


void tft_write_command(uint16_t command)
{
    *tft_register = command;
}

void tft_write_data(uint16_t data)
{
    tft_ram[0] = data;
}

void tft_delay(uint16_t delay)
{
	if (tft_normal_mode)
	{
		osDelay(delay);
	}
	else
	{
		for (uint64_t i = 0; i < (delay * 2800); i++);
	}
}

bool tft_buffer_copy()
{
    if (HAL_DMA_GetState(tft_dma) != HAL_DMA_STATE_READY)
    {
        HAL_DMA_Abort(tft_dma);
    }

    //DMA transfer length is in number of transactions!!!
    //chunks are set to 32b, because DMA transfer length is only 16-bit wide
    //DMA limit is therefore 4x65535 = 262,140 (we need 192,000)
    uint32_t len;
    len = (tft_win_x2 - tft_win_x1 + 1) * (tft_win_y2 - tft_win_y1 + 1);
    len /= 2;

    if (tft_controller_type == TFT_CONTROLLER_HX8352)
    {
        tft_set_window_hx8352(tft_win_x1, tft_win_y1, tft_win_x2, tft_win_y2);
    }
    else
    {
        tft_set_window_ili9327(tft_win_x1, tft_win_y1, tft_win_x2, tft_win_y2);
    }

    tft_dma_done = false;
    HAL_DMA_Start_IT(tft_dma, (uint32_t)tft_buffer_ptr, (uint32_t)tft_ram, len);

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

    if (tft_normal_mode)
    {
    	//osThreadFlagsSet(thread_gui, 0x01);
        gui_disp_ready();
    }
}

void tft_wait_for_buffer()
{
	if (tft_normal_mode)
	{
		while(!tft_dma_done);
	}
	else
	{
		tft_delay(20);
	}
}

void tft_refresh_buffer(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t * buffer)
{
	if (!tft_normal_mode)
	{
		tft_wait_for_buffer();
		tft_dma_done = false;
	}

    tft_win_x1 = x1;
    tft_win_y1 = y1;

    tft_win_x2 = x2;
    tft_win_y2 = y2;

    tft_buffer_ptr = buffer;

    tft_buffer_ready = true;

    if (tft_normal_mode)
    {
//    	osThreadFlagsWait(0x01, osFlagsWaitAny, WAIT_INF);
    }
    else
    {
    	tft_buffer_copy();
    }
}

void tft_init()
{
    //init vars
    tft_buffer_ready = false;
    tft_dma_done = true;

    //set pins
    GpioSetDirection(DISP_TE, INPUT, GPIO_NOPULL);

    //DMA callback
    tft_dma->XferCpltCallback = tft_irq_dma_done;

    //init controller
    tft_reset();

    //to dummy reads to prepare the controller
    tft_read_register(0x00);
    tft_read_register(0x00);
    uint8_t reg_00 = tft_read_register(0x00);

    INFO("reg 00 = %02X", reg_00);

    if (reg_00 == 0x65) //himax
    {
        INFO("Controller HX8352");
        tft_controller_type = TFT_CONTROLLER_HX8352;
        tft_init_hx8352();
    }
    else
    {
        INFO("Controller ILI9327");
        tft_controller_type = TFT_CONTROLLER_ILI9327;
        tft_init_ili9327();
    }
}

void tft_init_bsod()
{
	tft_normal_mode = false;
	if (tft_controller_type == TFT_CONTROLLER_NOT_INIT)
	{
		tft_init();
	}
	else
	{
		if (HAL_DMA_GetState(tft_dma) != HAL_DMA_STATE_READY)
		{
			HAL_DMA_Abort(tft_dma);
		}
	}
}

void tft_stop()
{
    HAL_GPIO_WritePin(DISP_RST, LOW);
}

void tft_reset()
{
    HAL_GPIO_WritePin(DISP_RST, HIGH);
    tft_delay(50);
    HAL_GPIO_WritePin(DISP_RST, LOW);
    tft_delay(100);
    HAL_GPIO_WritePin(DISP_RST, HIGH);
    tft_delay(100);
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
            if (x < 10)         r= 255;
            else if (x < 80)    r = y;
            else if (x < 90)    g = 255;
            else if (x < 160)   g = y;
            else if (x < 170)   b = 255;
            else                b = y;
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
