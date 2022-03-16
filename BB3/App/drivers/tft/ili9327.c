/*
 * hx8352.c
 *
 *  Created on: Apr 16, 2020
 *      Author: horinek
 *
 *  Frame buffer resides in main memory
 *  Display GRAM is mapped via FMC
 *  Data transfer starts during blanking time (when TE goes high) and buffer is modified
 *  Transfer is done via DMA, it must ends before TE goes low
 *  blanking time can be modified via reg 0x2b
 *
 */


#include "ili9327.h"

void tft_init_ili9327()
{
    tft_write_command(0x11); //Exit_sleep_mode

    tft_delay(120);

    tft_write_command(0xD1);//VCOM Control
    tft_write_data(0x00);
    tft_write_data(0x71);   //VCVM  VREG1OUT x 0.944
    tft_write_data(0x19);   //VDV   VREG1OUT x 1.20

    tft_write_command(0xD0);//Power_Setting
    tft_write_data(0x07);   //VC 1.0 x Vci
    tft_write_data(0x01);   //BT
    tft_write_data(0x08);

    tft_write_command(0x36);//set_address_mode
    tft_write_data(0xCA);

    tft_write_command(0x3A);//set_pixel_format
    tft_write_data(0x55);//DBI:16bit/pixel (65,536 colors)

    tft_write_command(0xC1);//Display_Timing_Setting for Normal/Partial Mode
    tft_write_data(0x10);
    tft_write_data(0x10);
    tft_write_data(0x02);
    tft_write_data(0x02);

    tft_write_command(0xC0); //Panel Driving Setting / Set Default Gamma
    tft_write_data(0x00);
    tft_write_data(0x35);
    tft_write_data(0x00);
    tft_write_data(0x00);
    tft_write_data(0x01);
    tft_write_data(0x02);

    tft_write_command(0xC5);  //Frame Rate Control / Set frame rate
    tft_write_data(0x04);//72Hz

    tft_write_command(0xD2); //Power_Setting for Normal Mode / power setting
    tft_write_data(0x01);//Gamma Driver Amplifier:1.00, Source Driver Amplifier: 1.00
    tft_write_data(0x44);

    tft_write_command(0xC8); //Gamma Setting / Set Gamma
    tft_write_data(0x04);
    tft_write_data(0x67);
    tft_write_data(0x35);
    tft_write_data(0x04);
    tft_write_data(0x08);
    tft_write_data(0x06);
    tft_write_data(0x24);
    tft_write_data(0x01);
    tft_write_data(0x37);
    tft_write_data(0x40);
    tft_write_data(0x03);
    tft_write_data(0x10);
    tft_write_data(0x08);
    tft_write_data(0x80);
    tft_write_data(0x00);

    tft_write_command(0x35);//set_tear_on
    tft_write_data(0x01);

    tft_color_fill(0xFFFF);
//	tft_test_pattern();
	tft_refresh_buffer(0, 0, 239, 399, tft_buffer);
	tft_wait_for_buffer();

    tft_write_command(0x29); //set_display_on / display on
}

void tft_set_window_ili9327(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    tft_write_command(0x2a);
    tft_write_data(x1 >> 8);
    tft_write_data(x1 & 0xFF);
    tft_write_data(x2 >> 8);
    tft_write_data(x2 & 0xFF);

    y1 += 16;
    y2 += 16;

    tft_write_command(0x2b);
    tft_write_data(y1 >> 8);
    tft_write_data(y1 & 0xFF);
    tft_write_data(y2 >> 8);
    tft_write_data(y2 & 0xFF);

    //Start write
    tft_write_command(0x2C);
}
