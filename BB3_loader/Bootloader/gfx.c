/*
 * gfx.cc
 *
 *  Created on: 18. 6. 2020
 *      Author: horinek
 *
 *
 */

#include "gfx.h"

#include "drivers/led.h"
#include "drivers/tft/tft.h"
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
#define GFX_BLACK   4
#define GFX_INVERT  5

uint8_t gfx_color;
uint8_t gfx_status;


#define BAT_X1  91
#define BAT_X2  151
#define BAT_Y1  156
#define BAT_Y2  255
#define BAT_W   (BAT_X2 - BAT_X1)
#define BAT_H   (BAT_Y2 - BAT_Y1)

uint8_t gfx_bg_init = GFX_NONE;

#define COLOR_WHITE 0xFFFF
#define COLOR_BLACK 0x0000
#define COLOR_GREEN 0x07E0

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

        if (gfx_color == GFX_INVERT)
        {
            value = ~value;
        }
        else
        {
            //565 to 888
            uint8_t r = (0b1111100000000000 & value) >> 8;
            uint8_t g = (0b0000011111100000 & value) >> 3;
            uint8_t b = (0b0000000000011111 & value) << 3;

            if (gfx_color != GFX_RED)
                r = r > alpha ? r - alpha : 0;

            if (gfx_color != GFX_GREEN)
                g = g > alpha ? g - alpha : 0;

            b = b > alpha ? b - alpha : 0;

            //convert 24bit to 16bit (565)
            r >>= 3;
            g >>= 2;
            b >>= 3;

            value = (r << 11) | (g << 5) | b;
        }
 		tft_buffer[pos] = value;

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
	if (gfx_bg_init == GFX_NONE)
		return;

    tft_wait_for_buffer();
    tft_color_fill(COLOR_WHITE);
    tft_refresh_buffer(0, 0, 239, 399);
}

void gfx_rect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    tft_wait_for_buffer();
    for (uint32_t y = y1; y < y2; y++)
    {
        for (uint32_t x = x1; x < x2; x++)
        {
            tft_buffer[y * TFT_WIDTH + x] = color;
        }
    }
}

void gfx_clear_part(uint16_t h)
{
    tft_wait_for_buffer();
    for (uint32_t i = 0; i < h * TFT_WIDTH; i++)
        tft_buffer[i] = COLOR_WHITE;
}

void gfx_draw_dot(int16_t x, int16_t y)
{
    gfx_draw_text(x, y, "7", MF_ALIGN_CENTER, gfx_icons);
}



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



void gfx_draw_status(uint8_t status, const char * message)
{
	if (gfx_bg_init == GFX_NONE)
	{
		tft_init();
	    led_set_backlight(10);

		gfx_text = mf_find_font("Roboto_Bold28");
		gfx_desc = mf_find_font("Roboto_Light28");
		gfx_icons = mf_find_font("icons");

		gfx_anim_init();

		srandom(0);
	}

	tft_wait_for_buffer();
	tft_color_fill(0xFFFF);
	gfx_bg_init = GFX_WHITE;

    char icon[3];
	char title[64];
    char text[64];
    char sub_text[64];

    icon[0] = 0;
	title[0] = 0;
    text[0] = 0;
    sub_text[0] = 0;

	uint8_t color = GFX_BLACK;
	gfx_status = status;

	switch (gfx_status)
	{
        case(GFX_STATUS_CHARGE_NONE):
            {
                strcpy(title, "Charging");
                gfx_get_charge_type(text);
            }
        break;
        case(GFX_STATUS_CHARGE_PASS):
            {
                strcpy(title, "Charging");
                gfx_get_charge_type(text);
            }
        break;
        case(GFX_STATUS_NONE_BOOST):
            {
                strcpy(title, "Power bank");
                if (pwr.boost_output == 0)
                {
                    strcpy(text, "connect device");
                }
                else
                {
                    strcpy(text, "output");
                    sprintf(sub_text, "%0.2f W", pwr.boost_output / 1000.0);
                }
            }
        break;
        case(GFX_STATUS_CHARGE_DATA):
            {
                strcpy(title, "USB mode");
                if (strlen(message) > 0)
                {
					strcpy(text, message);
					gfx_get_charge_type(sub_text);
                }
                else
                {
                	gfx_get_charge_type(text);
                }
            }
        break;
        case(GFX_STATUS_NONE_DATA):
            {
                strcpy(title, "USB mode");
                if (strlen(message) > 0)
                {
					strcpy(text, message);
					strcpy(sub_text, "Charging slow");
                }
                else
                {
                	strcpy(text, "Charging slow");
                }

            }
        break;
        case(GFX_STATUS_NONE_CHARGE):
            {
                strcpy(title, "Charging");
                strcpy(text, "Slow charging");
                strcpy(sub_text, "Switch ports!");
            }
        break;
        case(GFX_STATUS_NONE_NONE):
            {
            }
        break;
		case(GFX_STATUS_UPDATE):
			{
				strcpy(icon, "3");
				strcpy(title, "Updating");
				strcpy(text, message);
			}
		break;
		case(GFX_STATUS_ERROR):
			{
				strcpy(icon, "2");
				strcpy(title, "Error");
				strcpy(text, message);
				color = GFX_RED;
			}
		break;
		case(GFX_STATUS_SUCCESS):
			{
				strcpy(icon, "1");
				strcpy(title, "Success");
				strcpy(text, message);
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

	if (gfx_status >= GFX_STATUS_ANIMATED)
	{
	    gfx_color = GFX_BLACK;
        gfx_draw_text(TFT_WIDTH / 2, (TFT_HEIGHT / 2) - gfx_text->height * 6, title, MF_ALIGN_CENTER, gfx_text);
        gfx_draw_text(TFT_WIDTH / 2, (TFT_HEIGHT / 2) - gfx_text->height * 5, text, MF_ALIGN_CENTER, gfx_desc);
        gfx_draw_text(TFT_WIDTH / 2, (TFT_HEIGHT / 2) - gfx_text->height * 4, sub_text, MF_ALIGN_CENTER, gfx_desc);

        gfx_draw_text(TFT_WIDTH / 2, (TFT_HEIGHT / 2) - gfx_icons->height / 2, "6", MF_ALIGN_CENTER, gfx_icons);

        uint16_t val = BAT_Y1 + BAT_H - (BAT_H * pwr.fuel_gauge.battery_percentage) / 100;

        gfx_rect(BAT_X1, BAT_Y1, BAT_X2, val, COLOR_WHITE);
        gfx_rect(BAT_X1, val, BAT_X2, BAT_Y2, COLOR_GREEN);

        if (pwr.fuel_gauge.battery_percentage < 100)
            sprintf(text, "%u%%", pwr.fuel_gauge.battery_percentage);
        else
            strcpy(text, "Full");

        gfx_color = GFX_BLACK;
        gfx_draw_text(TFT_WIDTH / 2, BAT_Y1 + BAT_H / 2 - gfx_desc->height / 2, text, MF_ALIGN_CENTER, gfx_desc);

        tft_refresh_buffer(0, 0, 239, GFX_ANIM_TOP);
	}
	else
	{
        gfx_color = color;
        gfx_draw_text(TFT_WIDTH / 2, (TFT_HEIGHT / 2) - gfx_icons->height / 2 - 50, icon, MF_ALIGN_CENTER, gfx_icons);

        gfx_color = GFX_BLACK;
        gfx_draw_text(TFT_WIDTH / 2, TFT_HEIGHT - gfx_text->height * 3, title, MF_ALIGN_CENTER, gfx_text);
        gfx_draw_text(TFT_WIDTH / 2, TFT_HEIGHT - gfx_text->height * 2, text, MF_ALIGN_CENTER, gfx_desc);
        if (gfx_status == GFX_STATUS_UPDATE)
            tft_refresh_buffer(0, 0, 239, GFX_PROGRESS_TOP - 1);
        else
            tft_refresh_buffer(0, 0, 239, 399);
	}

}

void gfx_draw_progress(float val)
{
	if (val < 0) val = 0;
	if (val > 1) val = 1;

	uint16_t w = 239 * val;

    gfx_rect(0, 0, w, TFT_HEIGHT - GFX_PROGRESS_TOP, COLOR_GREEN);
    gfx_rect(w, 0, 239, TFT_HEIGHT - GFX_PROGRESS_TOP, COLOR_WHITE);

	tft_refresh_buffer(0, GFX_PROGRESS_TOP, 239, 399);
}

bool gfx_draw_anim()
{
    if (gfx_status < GFX_STATUS_ANIMATED)
        return true;

    gfx_clear_part(GFX_ANIM_BOTTOM - GFX_ANIM_TOP);

    char left_icon[2];
    char right_icon[3];

    switch (gfx_status)
    {
        case(GFX_STATUS_CHARGE_NONE):
            strcpy(left_icon, "4");
            strcpy(right_icon, "");
        break;
        case(GFX_STATUS_CHARGE_PASS):
            strcpy(left_icon, "4");
            strcpy(right_icon, "4");
        break;
        case(GFX_STATUS_NONE_BOOST):
            strcpy(left_icon, "");
            strcpy(right_icon, "4");
        break;
        case(GFX_STATUS_CHARGE_DATA):
            strcpy(left_icon, "4");
            strcpy(right_icon, "0");
        break;
        case(GFX_STATUS_NONE_DATA):
            strcpy(left_icon, "");
            strcpy(right_icon, "40");
        break;
        case(GFX_STATUS_NONE_CHARGE):
            strcpy(left_icon, "");
            strcpy(right_icon, "4");
        break;
        default:
            strcpy(left_icon, "");
            strcpy(right_icon, "");
    }

    gfx_color = GFX_GREEN;
    bool done = gfx_anim_step(gfx_status);

    gfx_color = GFX_BLACK;
    gfx_draw_text(10, TFT_HEIGHT - gfx_icons->height + 16 - GFX_ANIM_TOP, left_icon, MF_ALIGN_LEFT, gfx_icons);
    gfx_draw_text(TFT_WIDTH - 10, TFT_HEIGHT - gfx_icons->height  + 16 - GFX_ANIM_TOP, right_icon, MF_ALIGN_RIGHT, gfx_icons);

    if (development_mode)
    {
		if (gfx_status == GFX_STATUS_CHARGE_PASS || gfx_status == GFX_STATUS_NONE_BOOST)
		{
			char cc[8];
			char mode[4];

			if (pwr.data_usb_mode == dm_host_pass)
				strcpy(mode, "CDP");
			else
				strcpy(mode, "SDP");

			sprintf(cc, "%s %u%u", mode, (pwr.cc_conf & 0b10) >> 1, (pwr.cc_conf & 0b01));
			gfx_draw_text(TFT_WIDTH - 10, TFT_HEIGHT - GFX_ANIM_TOP - gfx_desc->height - 48, cc, MF_ALIGN_RIGHT, gfx_desc);
		}

		if (gfx_status == GFX_STATUS_NONE_BOOST)
		{
			char boost[8];
			sprintf(boost, "%0.2fV", 4.55 + pwr.boost_volt * 0.064);
			gfx_draw_text(TFT_WIDTH - 10, TFT_HEIGHT - GFX_ANIM_TOP - gfx_desc->height - 48 - 22, boost, MF_ALIGN_RIGHT, gfx_desc);
		}
    }

    tft_refresh_buffer(0, GFX_ANIM_TOP, 239, GFX_ANIM_BOTTOM - 1);

    return done;
}

void gfx_anim_wait()
{
    gfx_draw_status(GFX_STATUS_NONE_NONE, NULL);
    while(!gfx_draw_anim());
}
