/*
 * gfx.cc
 *
 *  Created on: 18. 6. 2020
 *      Author: horinek
 *
 *
 */

#include "gfx.h"

#include "drivers/tft_hx8352.h"
#include "lib/mcufont/mcufont.h"
#include "pwr_mng.h"

const struct mf_font_s * gfx_font;

const struct mf_font_s * gfx_icons;
const struct mf_font_s * gfx_text;
const struct mf_font_s * gfx_desc;

#define GFX_NONE	0
#define GFX_RED		1
#define GFX_GREEN	2
#define GFX_WHITE	3
#define GFX_BLACK	4

uint8_t gfx_color;

static void pixel_callback(int16_t x, int16_t y, uint8_t count, uint8_t alpha, void *state)
{
    uint32_t pos;
    uint16_t value;

    if (y < 0 || y >= TFT_HEIGHT) return;
    if (x < 0 || x + count >= TFT_WIDTH) return;

    while (count--)
    {
        pos = TFT_WIDTH * y + x;
        value = tft_buffer[pos];

        //565 to 888
 		uint8_t r = (0b1111100000000000 & value) >> 9;
 		uint8_t g = (0b0000011111100000 & value) >> 3;
 		uint8_t b = (0b0000000000011111 & value) << 2;

 		if (gfx_color != GFX_RED)
 			r = r > alpha ? r - alpha : 0;

 		if (gfx_color != GFX_GREEN)
 			g = g > alpha ? g - alpha : 0;

 		b = b > alpha ? b - alpha : 0;

 		//convert 24bit to 16bit (565)
 		r >>= 3;
 		g >>= 2;
 		b >>= 3;

 		tft_buffer[pos] = (r << 11) | (g << 5) | b;

        x++;
    }
}

static uint8_t character_callback(int16_t x, int16_t y, mf_char character, void *state)
{
    return mf_render_character(gfx_font, x, y, character, pixel_callback, state);
}

void gfx_draw_text(uint16_t x, uint16_t y, char * text, enum mf_align_t align, const struct mf_font_s * font)
{
	gfx_font = font;
	mf_render_aligned(gfx_font, x, y, align, text, strlen(text), character_callback, NULL);
}

void gfx_clear()
{
    tft_wait_for_buffer();
    tft_color_fill(0xFFFF);
    tft_refresh_buffer(0, 0, 239, 399);
}


uint8_t gfx_bg_init = GFX_NONE;

void gfx_get_charge_type(char * text)
{
    switch(pwr.charge_port)
    {
        case(PWR_CHARGE_NONE):
            strcpy(text, "");
        break;
        case(PWR_CHARGE_WEAK):
            strcpy(text, "Weak source!");
        break;
        case(PWR_CHARGE_SLOW):
            strcpy(text, "Charging slow");
        break;
        case(PWR_CHARGE_FAST):
            strcpy(text, "Charging fast");
        break;
        case(PWR_CHARGE_QUICK):
            strcpy(text, "Quick charge");
        break;
    }
}

void gfx_draw_status(uint8_t status, const char * sub_text)
{
	if (gfx_bg_init == GFX_NONE)
	{
		tft_init();
		tft_init_display();

		gfx_text = mf_find_font("Roboto_Bold28");
		gfx_desc = mf_find_font("Roboto_Light28");
		gfx_icons = mf_find_font("icons");
	}

	tft_wait_for_buffer();
	tft_color_fill(0xFFFF);
	gfx_bg_init = GFX_WHITE;

    char icon[2];
    char icon2[2];
	char title[64];
    char text[64];
    char text2[64];

    icon[0] = 0;
    icon2[0] = 0;
	title[0] = 0;
    text[0] = 0;
    text2[0] = 0;

	uint8_t color = GFX_BLACK;
	bool alt_mode = false;

	switch (status)
	{
        case(GFX_STATUS_CHARGE_NONE):
            {
                alt_mode = true;
                strcpy(icon, "4");
                strcpy(title, "Charging");
                gfx_get_charge_type(text);
            }
        break;
        case(GFX_STATUS_CHARGE_DATA):
            {
                alt_mode = true;
                strcpy(icon, "4");
                strcpy(icon2, "0");
                strcpy(title, "USB mode");
                strcpy(text, "Eject to start");
                gfx_get_charge_type(text2);
            }
        break;
        case(GFX_STATUS_NONE_DATA):
            {
                alt_mode = true;
                strcpy(icon2, "0");
                strcpy(title, "USB mode");
                strcpy(text, "Eject to start");
                strcpy(text2, "Charging slow");
            }
        break;
        case(GFX_STATUS_NONE_CHARGE):
            {
                alt_mode = true;
                strcpy(icon2, "4");
                strcpy(title, "Charging");
                strcpy(text, "Slow charging");
                strcpy(text2, "Switch ports!");
            }
        break;
		case(GFX_STATUS_UPDATE):
			{
				strcpy(icon, "3");
				strcpy(title, "Updating");
				strcpy(text, sub_text);
			}
		break;
		case(GFX_STATUS_ERROR):
			{
				strcpy(icon, "2");
				strcpy(title, "Error");
				strcpy(text, sub_text);
				color = GFX_RED;
			}
		break;
		case(GFX_STATUS_SUCCESS):
			{
				strcpy(icon, "1");
				strcpy(title, "Success");
				strcpy(text, sub_text);
				color = GFX_GREEN;
			}
		break;

        case(GFX_STATUS_TORCH):
            {
                strcpy(icon, "5");
                strcpy(title, "");
                strcpy(text, "");
            }
        break;

	}

	INFO("%s: %s", title, text);

	if (alt_mode)
	{
        gfx_draw_text(10, TFT_HEIGHT - gfx_icons->height, icon, MF_ALIGN_LEFT, gfx_icons);
        gfx_draw_text(TFT_WIDTH - 10, TFT_HEIGHT - gfx_icons->height, icon2, MF_ALIGN_RIGHT, gfx_icons);

        gfx_draw_text(TFT_WIDTH / 2, (TFT_HEIGHT / 2) - gfx_text->height * 2, title, MF_ALIGN_CENTER, gfx_text);
        gfx_draw_text(TFT_WIDTH / 2, (TFT_HEIGHT / 2) - gfx_text->height * 1, text, MF_ALIGN_CENTER, gfx_desc);
        gfx_draw_text(TFT_WIDTH / 2, (TFT_HEIGHT / 2), text2, MF_ALIGN_CENTER, gfx_desc);
	}
	else
	{
        gfx_color = color;
        gfx_draw_text(TFT_WIDTH / 2, (TFT_HEIGHT / 2) - gfx_icons->height / 2, icon, MF_ALIGN_CENTER, gfx_icons);

        gfx_color = GFX_BLACK;
        gfx_draw_text(TFT_WIDTH / 2, TFT_HEIGHT - gfx_text->height * 3, title, MF_ALIGN_CENTER, gfx_text);
        gfx_draw_text(TFT_WIDTH / 2, TFT_HEIGHT - gfx_text->height * 2, text, MF_ALIGN_CENTER, gfx_desc);
	}
	tft_refresh_buffer(0, 0, 239, 399);
}

void gfx_draw_progress(float val)
{
	uint16_t h = TFT_HEIGHT - gfx_text->height * 0.7;

	if (gfx_bg_init != GFX_BLACK)
	{
		tft_wait_for_buffer();
		tft_color_fill(0x0000);

		gfx_bg_init = GFX_BLACK;
	}

	if (val < 0) val = 0;
	if (val > 1) val = 1;

	tft_refresh_buffer(0, h, 239 * val, 399);
}
