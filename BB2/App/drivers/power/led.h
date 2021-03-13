/*
 * led.h
 *
 *  Created on: Oct 27, 2020
 *      Author: horinek
 */

#ifndef DRIVERS_LED_H_
#define DRIVERS_LED_H_

#include "../common.h"

void led_init();

void led_set_backlight(uint8_t val);
void led_set_torch(uint8_t val);

void led_period_irq();


#endif /* DRIVERS_LED_H_ */
