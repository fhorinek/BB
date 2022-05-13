/*
 * hx8352.c
 *
 *  Created on: Apr 16, 2020
 *      Author: horinek
 *
 */

#include "hx8352.h"

void tft_init_hx8352()
{
    // Register setting for EQ setting
    tft_write_register(0xe5, 0x10);      //
    tft_write_register(0xe7, 0x10);      //
    tft_write_register(0xe8, 0x48);      //
    tft_write_register(0xec, 0x09);      //
    tft_write_register(0xed, 0x6c);      //

    // Power on Setting
    tft_write_register(0x23, 0x6F);      //VMF
    tft_write_register(0x24, 0x57);      //VMH
    tft_write_register(0x25, 0x71);      //VML
    tft_write_register(0xE2, 0x18);      //
    tft_write_register(0x1B, 0x15);      //VRH
    tft_write_register(0x01, 0x00);      //
    tft_write_register(0x1C, 0x03);      //AP=3

    // Power on sequence
    tft_write_register(0x19, 0x01);      //OSCEN=1
    tft_delay(5);
    tft_write_register(0x1F, 0x8C);      //GASEN=1, DK=1, XDK=1
    tft_write_register(0x1F, 0x84);      //GASEN=1, XDK=1
    tft_delay(10);
    tft_write_register(0x1F, 0x94);      //GASEN=1, PON=1, XDK=1
    tft_delay(10);
    tft_write_register(0x1F, 0xD4);      //GASEN=1, VCOMG=1, PON=1, XDK=1
    tft_delay(5);

    tft_write_register(0x28, 0x20);      //GON=1
    tft_delay(40);
    tft_write_register(0x28, 0x38);      //GON=1, DTE=1, D=2
    tft_delay(40);

    tft_write_register(0x17, 0x05);		//COLMODE
    tft_write_register(0x60, 0x08);		//TE
    //tft_write_register(0x2b, 220);		//Blanking time (transfer time) Tearing control

	tft_write_register(0x16, 0x08);             //INVERTED PORTRAIT

	tft_color_fill(0xFFFF);
	tft_refresh_buffer(0, 0, 239, 399);
	tft_wait_for_buffer();

	tft_write_register(0x28, 0x3C); //display on //GON=1, DTE=1, D=3
}

NO_OPTI void tft_set_window_hx8352(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	tft_write_register(0x02, x1 >> 8);
	tft_write_register(0x03, x1 & 0xFF);

	tft_write_register(0x04, x2 >> 8);
	tft_write_register(0x05, x2 & 0xFF);

	tft_write_register(0x06, y1 >> 8);
	tft_write_register(0x07, y1 & 0xFF);

	tft_write_register(0x08, y2 >> 8);
	tft_write_register(0x09, y2 & 0xFF);

    tft_write_register(0x80, x1 >> 8);
    tft_write_register(0x81, x1);

    tft_write_register(0x82, y1 >> 8);
    tft_write_register(0x83, y1);

    //Start write
    tft_write_command(0x22);
}


