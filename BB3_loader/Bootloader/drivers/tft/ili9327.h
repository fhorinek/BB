/*
 * ili9327.h
 *
 *  Created on: 13. 4. 2021
 *      Author: horinek
 */

#ifndef DRIVERS_TFT_ILI9327_H_
#define DRIVERS_TFT_ILI9327_H_

#include "tft.h"

void tft_init_ili9327();
void tft_set_window_ili9327(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

#endif /* DRIVERS_TFT_ILI9327_H_ */
