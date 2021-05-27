/*
 * hx8352.h
 *
 *  Created on: Apr 16, 2020
 *      Author: horinek
 */

#ifndef TFT_HX8352_H_
#define TFT_HX8352_H_

#include "common.h"

#define TFT_WIDTH	240
#define TFT_HEIGHT	400
#define TFT_BUFFER_SIZE (TFT_WIDTH * TFT_HEIGHT)
extern uint16_t tft_buffer[TFT_BUFFER_SIZE];

void tft_init();
void tft_stop();
void tft_reset();

void tft_test_pattern();
void tft_color_fill(uint16_t color);

void tft_refresh_buffer(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void tft_wait_for_buffer();
void tft_irq_display_te();

#define TFT_CONTROLLER_HX8352       0
#define TFT_CONTROLLER_ILI9327      1

extern uint8_t tft_controller_type;

#endif /* TFT_HX8352_H_ */
