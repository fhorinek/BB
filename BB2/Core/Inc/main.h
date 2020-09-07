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
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
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
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal.h"

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

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define debug_uart huart7
#define esp_uart huart4
#define fanet_uart huart8
#define gnss_uart huart5
#define tft_dma hdma_memtomem_dma1_stream0
#define led_timmer htim2
#define led_bclk TIM_CHANNEL_1
#define led_torch TIM_CHANNEL_4
#define BT3_Pin GPIO_PIN_3
#define BT3_GPIO_Port GPIOE
#define REV_VCC_Pin GPIO_PIN_4
#define REV_VCC_GPIO_Port GPIOE
#define REV_0_Pin GPIO_PIN_5
#define REV_0_GPIO_Port GPIOE
#define REV_1_Pin GPIO_PIN_6
#define REV_1_GPIO_Port GPIOE
#define ESP_RESET_Pin GPIO_PIN_13
#define ESP_RESET_GPIO_Port GPIOC
#define CH_EN_OTG_Pin GPIO_PIN_0
#define CH_EN_OTG_GPIO_Port GPIOC
#define PWR_INT_Pin GPIO_PIN_1
#define PWR_INT_GPIO_Port GPIOC
#define ESP_BOOT_OPT_Pin GPIO_PIN_2
#define ESP_BOOT_OPT_GPIO_Port GPIOA
#define LED_TORCH_Pin GPIO_PIN_3
#define LED_TORCH_GPIO_Port GPIOA
#define VCC_MAIN_EN_Pin GPIO_PIN_4
#define VCC_MAIN_EN_GPIO_Port GPIOC
#define DISP_TE_Pin GPIO_PIN_5
#define DISP_TE_GPIO_Port GPIOC
#define DISP_TE_EXTI_IRQn EXTI9_5_IRQn
#define FN_RST_Pin GPIO_PIN_14
#define FN_RST_GPIO_Port GPIOB
#define GNSS_RST_Pin GPIO_PIN_15
#define GNSS_RST_GPIO_Port GPIOB
#define DISP_RST_Pin GPIO_PIN_12
#define DISP_RST_GPIO_Port GPIOD
#define ACC_INT_Pin GPIO_PIN_6
#define ACC_INT_GPIO_Port GPIOC
#define FN_BOOT0_Pin GPIO_PIN_10
#define FN_BOOT0_GPIO_Port GPIOA
#define DISP_BCKL_Pin GPIO_PIN_15
#define DISP_BCKL_GPIO_Port GPIOA
#define FN_EN_Pin GPIO_PIN_3
#define FN_EN_GPIO_Port GPIOD
#define BT5_Pin GPIO_PIN_6
#define BT5_GPIO_Port GPIOD
#define BT2_Pin GPIO_PIN_7
#define BT2_GPIO_Port GPIOD
#define BT4_Pin GPIO_PIN_5
#define BT4_GPIO_Port GPIOB
#define BT1_Pin GPIO_PIN_9
#define BT1_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

#define	ACC_INT			GPIOC, GPIO_PIN_6
#define	BT1				GPIOB, GPIO_PIN_9
#define	BT2				GPIOD, GPIO_PIN_7
#define	BT3				GPIOE, GPIO_PIN_3
#define	BT4				GPIOB, GPIO_PIN_5
#define	BT5				GPIOD, GPIO_PIN_6
#define	DISP_BCKL		GPIOA, GPIO_PIN_15
#define	DISP_RST		GPIOD, GPIO_PIN_12
#define	DISP_TE			GPIOC, GPIO_PIN_5
#define	ESP_BOOT_OPT	GPIOA, GPIO_PIN_2
#define	ESP_RESET		GPIOC, GPIO_PIN_13
#define	FN_BOOT0		GPIOA, GPIO_PIN_10
#define	FN_EN			GPIOD, GPIO_PIN_3
#define	FN_RST			GPIOB, GPIO_PIN_14
#define	GNSS_RST		GPIOB, GPIO_PIN_15
#define	CH_EN_OTG		GPIOC, GPIO_PIN_0
#define	LED_TORCH		GPIOA, GPIO_PIN_3
#define	PWR_INT			GPIOC, GPIO_PIN_1
#define	REV_0			GPIOE, GPIO_PIN_5
#define	REV_1			GPIOE, GPIO_PIN_6
#define	REV_VCC			GPIOE, GPIO_PIN_4
#define	VCC_MAIN_EN		GPIOC, GPIO_PIN_4




/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
