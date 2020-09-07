/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "../../App/debug.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
typedef StaticQueue_t osStaticMessageQDef_t;
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for System */
osThreadId_t SystemHandle;
uint32_t SystemBuffer[ 1024 ];
osStaticThreadDef_t SystemControlBlock;
const osThreadAttr_t System_attributes = {
  .name = "System",
  .stack_mem = &SystemBuffer[0],
  .stack_size = sizeof(SystemBuffer),
  .cb_mem = &SystemControlBlock,
  .cb_size = sizeof(SystemControlBlock),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Debug */
osThreadId_t DebugHandle;
uint32_t DebugBuffer[ 512 ];
osStaticThreadDef_t DebugControlBlock;
const osThreadAttr_t Debug_attributes = {
  .name = "Debug",
  .stack_mem = &DebugBuffer[0],
  .stack_size = sizeof(DebugBuffer),
  .cb_mem = &DebugControlBlock,
  .cb_size = sizeof(DebugControlBlock),
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for GUI */
osThreadId_t GUIHandle;
uint32_t GUIBuffer[ 1024 ];
osStaticThreadDef_t GUIControlBlock;
const osThreadAttr_t GUI_attributes = {
  .name = "GUI",
  .stack_mem = &GUIBuffer[0],
  .stack_size = sizeof(GUIBuffer),
  .cb_mem = &GUIControlBlock,
  .cb_size = sizeof(GUIControlBlock),
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for queue_Debug */
osMessageQueueId_t queue_DebugHandle;
uint8_t queue_DebugBuffer[ 16 * sizeof( debug_msg_t ) ];
osStaticMessageQDef_t queue_DebugControlBlock;
const osMessageQueueAttr_t queue_Debug_attributes = {
  .name = "queue_Debug",
  .cb_mem = &queue_DebugControlBlock,
  .cb_size = sizeof(queue_DebugControlBlock),
  .mq_mem = &queue_DebugBuffer,
  .mq_size = sizeof(queue_DebugBuffer)
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void task_System(void *argument);
extern void task_Debug(void *argument);
extern void task_GUI(void *argument);

extern void MX_USB_DEVICE_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of queue_Debug */
  queue_DebugHandle = osMessageQueueNew (16, sizeof(debug_msg_t), &queue_Debug_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of System */
  SystemHandle = osThreadNew(task_System, NULL, &System_attributes);

  /* creation of Debug */
  DebugHandle = osThreadNew(task_Debug, NULL, &Debug_attributes);

  /* creation of GUI */
  GUIHandle = osThreadNew(task_GUI, NULL, &GUI_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_task_System */
/**
  * @brief  Function implementing the System thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_task_System */
__weak void task_System(void *argument)
{
  /* init code for USB_DEVICE */
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN task_System */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END task_System */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
