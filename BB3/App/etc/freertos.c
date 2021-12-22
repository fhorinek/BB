/*
 * freertos.c
 *
 *  Created on: Feb 11, 2021
 *      Author: horinek
 */
#include "common.h"

uint16_t rtos_counter = 0;

void rtos_timer_elapsed()
{
    rtos_counter++;
}

void configureTimerForRunTimeStats(void)
{
    HAL_TIM_Base_Start_IT(rtos_timer);
}

unsigned long getRunTimeCounterValue(void)
{
    uint32_t cnt = (rtos_counter << 16) | __HAL_TIM_GET_COUNTER(rtos_timer);
    return cnt;
}

//void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
//{
//    FAULT("RTOS Task '%s' stack overflow!", pcTaskName);
//    Error_Handler();
//}
