/*
 * led.h
 *
 *  Created on: Oct 27, 2020
 *      Author: horinek
 */

#ifndef DRIVERS_LED_H_
#define DRIVERS_LED_H_

#include "common.h"

void led_init();

void led_set_backlight(uint8_t val);
void led_set_backlight_timeout(uint16_t timeout);
void led_set_torch(uint8_t val);


void led_period_irq();
void disp_period_irq();

void led_dim();

#endif /* DRIVERS_LED_H_ */
