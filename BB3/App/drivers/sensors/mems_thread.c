/*
 * mems.c
 *
 *  Created on: Sep 14, 2020
 *      Author: horinek
 */

#define DEBUG_LEVEL DBG_DEBUG

#include "mems_thread.h"

#include "drivers/sensors/ms5611.h"
#include "drivers/sensors/lsm9ds1.h"

#include "fc/imu.h"
#include "fc/vario.h"

#include "fc/fc.h"

//Mems
//1E - acc + mag
//6B - gyro
//77 - ms8266

//system
//36 - fuel gauge
//44 - light
//6A - charger



void thread_mems_start(void *argument)
{
	INFO("MEMS started");

    ms5611_init();
    lsm_init();
    imu_init();
    vario_init();

    if (fc.baro.status != fc_dev_ready)
        osThreadSuspend(thread_mems);

    ms5611_StartPressure(NULL);
    osDelay(10);

	HAL_TIM_OC_Start_IT(meas_timer, TIM_CHANNEL_1);
	HAL_TIM_OC_Start_IT(meas_timer, TIM_CHANNEL_2);

	while(!system_power_off)
	{
	    osThreadFlagsWait(0x01, osFlagsWaitAny, WAIT_INF);
	    imu_step();
	    vario_step();
//        DBG("Mems irq %u", GpioRead(ACC_INT));
//        DBG("Pres");
//        DBG(" %0.2fPa", fc.vario.pressure);
//        DBG("ACC");
//        DBG(" X %d", fc.imu.raw.acc.x);
//        DBG(" Y %d", fc.imu.raw.acc.y);
//        DBG(" Z %d", fc.imu.raw.acc.z);
//        DBG("GYRO");
//        DBG(" X %d", fc.imu.raw.gyro.x);
//        DBG(" Y %d", fc.imu.raw.gyro.y);
//        DBG(" Z %d", fc.imu.raw.gyro.z);
//        DBG("MAG");
//        DBG(" X %d", fc.imu.raw.mag.x);
//        DBG(" Y %d", fc.imu.raw.mag.y);
//        DBG(" Z %d", fc.imu.raw.mag.z);
//        DBG("\n");
	}

    INFO("Done");
    osThreadSuspend(thread_mems);
}

void mems_phase1();
void mems_phase1_1();

void mems_phase2();
void mems_phase2_1();
void mems_phase2_2();
void mems_phase2_3();
void mems_phase2_4();

void mems_phase1()     //t = 0
{
    mems_i2c_wait();

    //start reading raw_pressure
    ms5611_ReadPressure(mems_phase1_1);
}

void mems_phase1_1()    //pressure read completed
{
    //start temperature measurement
    ms5611_StartTemperature(NULL);
    fc.baro.pressure = ms5611_CompensatePressure();
}

void mems_phase2()		//t = 0.78ms
{
    mems_i2c_wait();

    //start reading temperature
	ms5611_ReadTemperature(mems_phase2_1);
}

void mems_phase2_1()    //temperature read complete
{
    //start pressure measurement
	ms5611_StartPressure(mems_phase2_2);

    ms5611_CompensateTemperature();
}

void mems_phase2_2()    //start pressure measurement command send
{
	if (fc.imu.status != fc_dev_error)
	{
	    //start reading fifo (acc & gyro)
	    lsm_fifo_start(mems_phase2_3);
	}
	else
	{
	    //notify mems loop that we have new data to process
	    osThreadFlagsSet(thread_mems, 0x01);
	}
}

void mems_phase2_3()    //fifo data were loaded to buffer
{
    //load data from transfer buffer
    lsm_read_fifo(&fc.imu.raw.acc, &fc.imu.raw.gyro);
    //start reading magnetometer
    lsm_mag_start(mems_phase2_4);
}

void mems_phase2_4()    //mag data  were loaded to buffer
{
    //load data from transfer buffer
    lsm_read_vector(&fc.imu.raw.mag);
    //notify mems loop that we have new data to process
    osThreadFlagsSet(thread_mems, 0x01);
}

