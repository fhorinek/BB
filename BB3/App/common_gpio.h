/*
 * common_gpio.h
 *
 *  Created on: 11. 8. 2020
 *      Author: horinek
 */

#ifndef COMMON_GPIO_H_
#define COMMON_GPIO_H_


#define	PA0		GPIOA,	GPIO_PIN_0
#define	PA1		GPIOA,	GPIO_PIN_1
#define	PA2		GPIOA,	GPIO_PIN_2
#define	PA3		GPIOA,	GPIO_PIN_3
#define	PA4		GPIOA,	GPIO_PIN_4
#define	PA5		GPIOA,	GPIO_PIN_5
#define	PA6		GPIOA,	GPIO_PIN_6
#define	PA7		GPIOA,	GPIO_PIN_7
#define	PA8		GPIOA,	GPIO_PIN_8
#define	PA9		GPIOA,	GPIO_PIN_9
#define	PA10	GPIOA,	GPIO_PIN_10
#define	PA11	GPIOA,	GPIO_PIN_11
#define	PA12	GPIOA,	GPIO_PIN_12
#define	PA13	GPIOA,	GPIO_PIN_13
#define	PA14	GPIOA,	GPIO_PIN_14
#define	PA15	GPIOA,	GPIO_PIN_15
#define	PB0		GPIOB,	GPIO_PIN_0
#define	PB1		GPIOB,	GPIO_PIN_1
#define	PB2		GPIOB,	GPIO_PIN_2
#define	PB3		GPIOB,	GPIO_PIN_3
#define	PB4		GPIOB,	GPIO_PIN_4
#define	PB5		GPIOB,	GPIO_PIN_5
#define	PB6		GPIOB,	GPIO_PIN_6
#define	PB7		GPIOB,	GPIO_PIN_7
#define	PB8		GPIOB,	GPIO_PIN_8
#define	PB9		GPIOB,	GPIO_PIN_9
#define	PB10	GPIOB,	GPIO_PIN_10
#define	PB11	GPIOB,	GPIO_PIN_11
#define	PB12	GPIOB,	GPIO_PIN_12
#define	PB13	GPIOB,	GPIO_PIN_13
#define	PB14	GPIOB,	GPIO_PIN_14
#define	PB15	GPIOB,	GPIO_PIN_15
#define	PC0		GPIOC,	GPIO_PIN_0
#define	PC1		GPIOC,	GPIO_PIN_1
#define	PC2		GPIOC,	GPIO_PIN_2
#define	PC3		GPIOC,	GPIO_PIN_3
#define	PC4		GPIOC,	GPIO_PIN_4
#define	PC5		GPIOC,	GPIO_PIN_5
#define	PC6		GPIOC,	GPIO_PIN_6
#define	PC7		GPIOC,	GPIO_PIN_7
#define	PC8		GPIOC,	GPIO_PIN_8
#define	PC9		GPIOC,	GPIO_PIN_9
#define	PC10	GPIOC,	GPIO_PIN_10
#define	PC11	GPIOC,	GPIO_PIN_11
#define	PC12	GPIOC,	GPIO_PIN_12
#define	PC13	GPIOC,	GPIO_PIN_13
#define	PC14	GPIOC,	GPIO_PIN_14
#define	PC15	GPIOC,	GPIO_PIN_15
#define	PD0		GPIOD,	GPIO_PIN_0
#define	PD1		GPIOD,	GPIO_PIN_1
#define	PD2		GPIOD,	GPIO_PIN_2
#define	PD3		GPIOD,	GPIO_PIN_3
#define	PD4		GPIOD,	GPIO_PIN_4
#define	PD5		GPIOD,	GPIO_PIN_5
#define	PD6		GPIOD,	GPIO_PIN_6
#define	PD7		GPIOD,	GPIO_PIN_7
#define	PD8		GPIOD,	GPIO_PIN_8
#define	PD9		GPIOD,	GPIO_PIN_9
#define	PD10	GPIOD,	GPIO_PIN_10
#define	PD11	GPIOD,	GPIO_PIN_11
#define	PD12	GPIOD,	GPIO_PIN_12
#define	PD13	GPIOD,	GPIO_PIN_13
#define	PD14	GPIOD,	GPIO_PIN_14
#define	PD15	GPIOD,	GPIO_PIN_15
#define	PE0		GPIOE,	GPIO_PIN_0
#define	PE1		GPIOE,	GPIO_PIN_1
#define	PE2		GPIOE,	GPIO_PIN_2
#define	PE3		GPIOE,	GPIO_PIN_3
#define	PE4		GPIOE,	GPIO_PIN_4
#define	PE5		GPIOE,	GPIO_PIN_5
#define	PE6		GPIOE,	GPIO_PIN_6
#define	PE7		GPIOE,	GPIO_PIN_7
#define	PE8		GPIOE,	GPIO_PIN_8
#define	PE9		GPIOE,	GPIO_PIN_9
#define	PE10	GPIOE,	GPIO_PIN_10
#define	PE11	GPIOE,	GPIO_PIN_11
#define	PE12	GPIOE,	GPIO_PIN_12
#define	PE13	GPIOE,	GPIO_PIN_13
#define	PE14	GPIOE,	GPIO_PIN_14
#define	PE15	GPIOE,	GPIO_PIN_15

#define PIN(A, B)	B
#define PORT(A, B)	A

#define HIGH	GPIO_PIN_SET
#define LOW		GPIO_PIN_RESET

#define INPUT	GPIO_MODE_INPUT
#define OUTPUT	GPIO_MODE_OUTPUT_PP

#define GpioWrite(pin, level)	HAL_GPIO_WritePin(pin, level)
#define GpioRead(pin) 			HAL_GPIO_ReadPin(pin)
void GpioSetDirection(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint16_t direction, uint16_t pull);


#endif /* COMMON_GPIO_H_ */
