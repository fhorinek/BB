/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define FANET_RESET_Pin GPIO_PIN_2
#define FANET_RESET_GPIO_Port GPIOE
#define FANET_BOOT0_Pin GPIO_PIN_3
#define FANET_BOOT0_GPIO_Port GPIOE
#define FANET_SW_EN_Pin GPIO_PIN_4
#define FANET_SW_EN_GPIO_Port GPIOE
#define TP10_Pin GPIO_PIN_5
#define TP10_GPIO_Port GPIOE
#define TP11_Pin GPIO_PIN_6
#define TP11_GPIO_Port GPIOE
#define TP13_Pin GPIO_PIN_13
#define TP13_GPIO_Port GPIOC
#define TP5_Pin GPIO_PIN_0
#define TP5_GPIO_Port GPIOC
#define GPS_SW_EN_Pin GPIO_PIN_1
#define GPS_SW_EN_GPIO_Port GPIOC
#define GPS_PPS_Pin GPIO_PIN_2
#define GPS_PPS_GPIO_Port GPIOC
#define GPS_RESET_Pin GPIO_PIN_3
#define GPS_RESET_GPIO_Port GPIOC
#define RGB_GREEN_Pin GPIO_PIN_2
#define RGB_GREEN_GPIO_Port GPIOA
#define RGB_BLUE_Pin GPIO_PIN_3
#define RGB_BLUE_GPIO_Port GPIOA
#define BT5_Pin GPIO_PIN_4
#define BT5_GPIO_Port GPIOA
#define DISPLAY_BCK_PWM_Pin GPIO_PIN_5
#define DISPLAY_BCK_PWM_GPIO_Port GPIOA
#define BT4_Pin GPIO_PIN_6
#define BT4_GPIO_Port GPIOA
#define BT3_Pin GPIO_PIN_7
#define BT3_GPIO_Port GPIOA
#define TP6_Pin GPIO_PIN_4
#define TP6_GPIO_Port GPIOC
#define TP12_Pin GPIO_PIN_5
#define TP12_GPIO_Port GPIOC
#define TP7_Pin GPIO_PIN_0
#define TP7_GPIO_Port GPIOB
#define DISPLAY_TE_Pin GPIO_PIN_1
#define DISPLAY_TE_GPIO_Port GPIOB
#define DISPLAY_TE_EXTI_IRQn EXTI1_IRQn
#define TP9_Pin GPIO_PIN_2
#define TP9_GPIO_Port GPIOB
#define ESP_BOOT_OPTION_Pin GPIO_PIN_12
#define ESP_BOOT_OPTION_GPIO_Port GPIOD
#define ESP_EN_RESET_Pin GPIO_PIN_13
#define ESP_EN_RESET_GPIO_Port GPIOD
#define DISPLAY_RESET_Pin GPIO_PIN_6
#define DISPLAY_RESET_GPIO_Port GPIOC
#define ESP_SW_EN_Pin GPIO_PIN_9
#define ESP_SW_EN_GPIO_Port GPIOA
#define CDMMC1_SW_EN_Pin GPIO_PIN_3
#define CDMMC1_SW_EN_GPIO_Port GPIOD
#define SDMMC1_CDET_Pin GPIO_PIN_6
#define SDMMC1_CDET_GPIO_Port GPIOD
#define DC_DC_BOOST_EN_Pin GPIO_PIN_7
#define DC_DC_BOOST_EN_GPIO_Port GPIOD
#define RGB_RED_Pin GPIO_PIN_3
#define RGB_RED_GPIO_Port GPIOB
#define CHARGER_DISABLE_Pin GPIO_PIN_4
#define CHARGER_DISABLE_GPIO_Port GPIOB
#define BT1_Pin GPIO_PIN_5
#define BT1_GPIO_Port GPIOB
#define MEMS_LDO_EN_Pin GPIO_PIN_8
#define MEMS_LDO_EN_GPIO_Port GPIOB
#define BT2_Pin GPIO_PIN_9
#define BT2_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */
#define	FANET_RESET			FANET_RESET_GPIO_Port, FANET_RESET_Pin
#define	FANET_BOOT0			FANET_BOOT0_GPIO_Port, FANET_BOOT0_Pin
#define	FANET_SW_EN			FANET_SW_EN_GPIO_Port, FANET_SW_EN_Pin
#define	TP10				TP10_GPIO_Port, TP10_Pin
#define	TP11				TP11_GPIO_Port, TP11_Pin
#define	TP13				TP13_GPIO_Port, TP13_Pin
#define	TP5					TP5_GPIO_Port, TP5_Pin
#define	GPS_SW_EN			GPS_SW_EN_GPIO_Port, GPS_SW_EN_Pin
#define	GPS_PPS				GPS_PPS_GPIO_Port, GPS_PPS_Pin
#define	GPS_RESET			GPS_RESET_GPIO_Port, GPS_RESET_Pin
#define	GPS_TX				GPS_TX_GPIO_Port, GPS_TX_Pin
#define	GPS_RX				GPS_RX_GPIO_Port, GPS_RX_Pin
#define	RGB_LED_GREEN		RGB_GREEN_GPIO_Port, RGB_GREEN_Pin
#define	RGB_LED_BLUE		RGB_BLUE_GPIO_Port, RGB_BLUE_Pin
#define	BT5					BT5_GPIO_Port, BT5_Pin
#define	DISPLAY_BCK_PWM		DISPLAY_BCK_PWM_GPIO_Port, DISPLAY_BCK_PWM_Pin
#define	BT4					BT4_GPIO_Port, BT4_Pin
#define	BT3					BT3_GPIO_Port, BT3_Pin
#define	TP6					TP6_GPIO_Port, TP6_Pin
#define	TP12				TP12_GPIO_Port, TP12_Pin
#define	TP7					TP7_GPIO_Port, TP7_Pin
#define	DISPLAY_TE			DISPLAY_TE_GPIO_Port, DISPLAY_TE_Pin
#define	TP9					TP9_GPIO_Port, TP9_Pin
#define	ESP_BOOT_OPTION		ESP_BOOT_OPTION_GPIO_Port, ESP_BOOT_OPTION_Pin
#define	ESP_EN_RESET		ESP_EN_RESET_GPIO_Port, ESP_EN_RESET_Pin
#define	DISPLAY_RESET		DISPLAY_RESET_GPIO_Port, DISPLAY_RESET_Pin
#define	ESP_TX				ESP_TX_GPIO_Port, ESP_TX_Pin
#define	ESP_SW_EN			ESP_SW_EN_GPIO_Port, ESP_SW_EN_Pin
#define	ESP_TXA15			ESP_TXA15_GPIO_Port, ESP_TXA15_Pin
#define	CDMMC1_SW_EN		CDMMC1_SW_EN_GPIO_Port, CDMMC1_SW_EN_Pin
#define	SDMMC1_CDET			SDMMC1_CDET_GPIO_Port, SDMMC1_CDET_Pin
#define	DC_DC_BOOST_EN		DC_DC_BOOST_EN_GPIO_Port, DC_DC_BOOST_EN_Pin
#define	RGB_LED_RED			RGB_RED_GPIO_Port, RGB_RED_Pin
#define	CHARGER_DISABLE		CHARGER_DISABLE_GPIO_Port, CHARGER_DISABLE_Pin
#define	BT1					BT1_GPIO_Port, BT1_Pin
#define	BARO_SCL			BARO_SCL_GPIO_Port, BARO_SCL_Pin
#define	BARO_SDA			BARO_SDA_GPIO_Port, BARO_SDA_Pin
#define	MEMS_LDO_EN			MEMS_LDO_EN_GPIO_Port, MEMS_LDO_EN_Pin
#define	BT2					BT2_GPIO_Port, BT2_Pin
#define	FANET_RX			FANET_RX_GPIO_Port, FANET_RX_Pin
#define	FANET_TX			FANET_TX_GPIO_Port, FANET_TX_Pin

#define HIGH	GPIO_PIN_SET
#define LOW		GPIO_PIN_RESET


/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
