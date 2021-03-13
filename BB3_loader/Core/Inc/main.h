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
#define disp_timer (&htim2)
#define esp_uart (&huart4)
#define debug_uart (&huart7)
#define fanet_uart (&huart8)
#define gnss_uart (&huart5)
#define tft_dma (&hdma_memtomem_dma1_stream0)
#define led_timer (&htim15)
#define led_bclk TIM_CHANNEL_4
#define led_torch TIM_CHANNEL_2
#define mems_i2c (&hi2c1)
#define sys_i2c (&hi2c2)
#define meas_timer (&htim3)
#define esp_spi (&hspi1)
#define rtos_timer (&htim17)
#define BT3_Pin GPIO_PIN_3
#define BT3_GPIO_Port GPIOE
#define BT3_EXTI_IRQn EXTI3_IRQn
#define USB_DATA_DFP_3A_Pin GPIO_PIN_4
#define USB_DATA_DFP_3A_GPIO_Port GPIOE
#define USB_DATA_DFP_1A_Pin GPIO_PIN_5
#define USB_DATA_DFP_1A_GPIO_Port GPIOE
#define LED_TORCH_Pin GPIO_PIN_6
#define LED_TORCH_GPIO_Port GPIOE
#define ALT_CH_EN_Pin GPIO_PIN_13
#define ALT_CH_EN_GPIO_Port GPIOC
#define BQ_OTG_Pin GPIO_PIN_0
#define BQ_OTG_GPIO_Port GPIOC
#define BQ_INT_Pin GPIO_PIN_1
#define BQ_INT_GPIO_Port GPIOC
#define BQ_INT_EXTI_IRQn EXTI1_IRQn
#define ESP_BOOT_Pin GPIO_PIN_2
#define ESP_BOOT_GPIO_Port GPIOA
#define DISP_BCKL_Pin GPIO_PIN_3
#define DISP_BCKL_GPIO_Port GPIOA
#define VCC_MAIN_EN_Pin GPIO_PIN_4
#define VCC_MAIN_EN_GPIO_Port GPIOC
#define DISP_TE_Pin GPIO_PIN_5
#define DISP_TE_GPIO_Port GPIOC
#define DISP_TE_EXTI_IRQn EXTI9_5_IRQn
#define DISP_RST_Pin GPIO_PIN_12
#define DISP_RST_GPIO_Port GPIOD
#define USB_VBUS_Pin GPIO_PIN_9
#define USB_VBUS_GPIO_Port GPIOA
#define ESP_EN_Pin GPIO_PIN_10
#define ESP_EN_GPIO_Port GPIOA
#define NG_CDP_CLM_Pin GPIO_PIN_15
#define NG_CDP_CLM_GPIO_Port GPIOA
#define FANET_SW_Pin GPIO_PIN_3
#define FANET_SW_GPIO_Port GPIOD
#define BT1_Pin GPIO_PIN_6
#define BT1_GPIO_Port GPIOD
#define BT1_EXTI_IRQn EXTI9_5_IRQn
#define BT4_Pin GPIO_PIN_7
#define BT4_GPIO_Port GPIOD
#define BT2_Pin GPIO_PIN_5
#define BT2_GPIO_Port GPIOB
#define BT5_Pin GPIO_PIN_9
#define BT5_GPIO_Port GPIOB
#define BT5_EXTI_IRQn EXTI9_5_IRQn
/* USER CODE BEGIN Private defines */

#define ALT_CH_EN           ALT_CH_EN_GPIO_Port, ALT_CH_EN_Pin
#define BQ_INT              BQ_INT_GPIO_Port, BQ_INT_Pin
#define BQ_OTG              BQ_OTG_GPIO_Port, BQ_OTG_Pin
#define BT1                 BT1_GPIO_Port, BT1_Pin
#define BT2                 BT2_GPIO_Port, BT2_Pin
#define BT3                 BT3_GPIO_Port, BT3_Pin
#define BT4                 BT4_GPIO_Port, BT4_Pin
#define BT5                 BT5_GPIO_Port, BT5_Pin
#define DISP_BCKL           DISP_BCKL_GPIO_Port, DISP_BCKL_Pin
#define DISP_RST            DISP_RST_GPIO_Port, DISP_RST_Pin
#define DISP_TE             DISP_TE_GPIO_Port, DISP_TE_Pin
#define ESP_BOOT            ESP_BOOT_GPIO_Port, ESP_BOOT_Pin
#define ESP_EN              ESP_EN_GPIO_Port, ESP_EN_Pin
#define FANET_BOOT0         FANET_BOOT0_GPIO_Port, FANET_BOOT0_Pin
#define FANET_RST           FANET_RST_GPIO_Port, FANET_RST_Pin
#define FANET_SW            FANET_SW_GPIO_Port, FANET_SW_Pin
#define GNSS_RST            GNSS_RST_GPIO_Port, GNSS_RST_Pin
#define LED_TORCH           LED_TORCH_GPIO_Port, LED_TORCH_Pin
#define NG_CDP_CLM          NG_CDP_CLM_GPIO_Port, NG_CDP_CLM_Pin
#define USB_DATA_DFP_1A     USB_DATA_DFP_1A_GPIO_Port, USB_DATA_DFP_1A_Pin
#define USB_DATA_DFP_3A     USB_DATA_DFP_3A_GPIO_Port, USB_DATA_DFP_3A_Pin
#define USB_VBUS            USB_VBUS_GPIO_Port, USB_VBUS_Pin
#define VCC_MAIN_EN         VCC_MAIN_EN_GPIO_Port, VCC_MAIN_EN_Pin

#define REV_0               GPIOC, GPIO_PIN_2
#define REV_1               GPIOC, GPIO_PIN_3

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
