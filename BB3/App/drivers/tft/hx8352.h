/*
 * hx8352.h
 *
 *  Created on: 13. 4. 2021
 *      Author: horinek
 */

#ifndef DRIVERS_TFT_HX8352_H_
#define DRIVERS_TFT_HX8352_H_

#include "tft.h"

void tft_init_hx8352();
void tft_set_window_hx8352(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

#endif /* DRIVERS_TFT_HX8352_H_ */
