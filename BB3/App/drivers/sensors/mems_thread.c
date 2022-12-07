/*
 * mems.c
 *
 *  Created on: Sep 14, 2020
 *      Author: horinek
 */

#define DEBUG_LEVEL DBG_DEBUG

#include "mems_thread.h"

#include "drivers/sensors/ms5611.h"
#include <drivers/sensors/lsm/lsm.h>

#include "fc/imu.h"
#include "fc/vario.h"

#include "fc/fc.h"

#include <gui/tasks/menu/system/calibration/calibration.h>

//Mems
//1E - acc + mag
//6B - gyro
//77 - ms8266

//system
//36 - fuel gauge
//44 - light
//6A - charger

ms_sensor_data_t ms_primary = {.addr  = MS5611_PRIMARY_ADDR};
ms_sensor_data_t ms_aux = {.addr  = MS5611_AUX_ADDR};;

void thread_mems_start(void *argument)
{
    UNUSED(argument);

    system_wait_for_handle(&thread_mems);

	INFO("MEMS started");

	fc.baro.retry_cnt = 0;
	do
	{
		fc.baro.status = ms5611_init(&ms_primary) ? fc_dev_ready : fc_dev_error;
		fc.aux_baro.status = ms5611_init(&ms_aux) ? fc_dev_ready : fc_dev_error;
		osDelay(10);
		fc.baro.retry_cnt++;
	} while(fc.baro.status != fc_dev_ready && fc.baro.retry_cnt < 10);

    lsm_init();
    imu_init();
    vario_init();

//    if (fc.baro.status != fc_dev_ready)
//        osThreadSuspend(thread_mems);

	HAL_TIM_OC_Start_IT(meas_timer, TIM_CHANNEL_1);
	HAL_TIM_OC_Start_IT(meas_timer, TIM_CHANNEL_2);

	while(!system_power_off)
	{
	    osThreadFlagsWait(0x01, osFlagsWaitAny, WAIT_INF);
	    imu_step();
	    vario_step();

	    if (gui.task.actual == &gui_calibration)
	    	calibration_imu_cb();

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
void mems_phase1_1_1();
void mems_phase1_1_2();

void mems_phase2();
void mems_phase2_1();
void mems_phase2_2();
void mems_phase2_2_1();
void mems_phase2_2_2();
void mems_phase2_3();
void mems_phase2_4();
void mems_phase2_5();

void mems_phase1()     //t = 0
{
    mems_i2c_wait();

    //start reading raw_pressure
	if (fc.baro.status == fc_dev_ready)
		ms5611_ReadPressure(&ms_primary, mems_phase1_1);
	else
		mems_phase1_1_1();
}

void mems_phase1_1()    //pressure read completed
{
    //start temperature measurement
    ms5611_StartTemperature(&ms_primary, mems_phase1_1_1);
    fc.baro.pressure = ms5611_CompensatePressure(&ms_primary);
}

// --- aux baro --
void mems_phase1_1_1()
{
    if (fc.aux_baro.status == fc_dev_ready)
        ms5611_ReadPressure(&ms_aux, mems_phase1_1_2);
}

void mems_phase1_1_2()
{
    ms5611_StartTemperature(&ms_aux, NULL);
    fc.aux_baro.pressure = ms5611_CompensatePressure(&ms_aux);
}
// ----------


void mems_phase2()		//t = 0.78ms
{
    mems_i2c_wait();

    //start reading temperature
    if (fc.baro.status == fc_dev_ready)
    	ms5611_ReadTemperature(&ms_primary, mems_phase2_1);
    else if (fc.aux_baro.status == fc_dev_ready)
    	mems_phase2_2_1();
    else
    	mems_phase2_2();
}

void mems_phase2_1()    //temperature read complete
{
    //start pressure measurement
    if (fc.aux_baro.status == fc_dev_ready)
        ms5611_StartPressure(&ms_primary, mems_phase2_2_1);
    else
        ms5611_StartPressure(&ms_primary, mems_phase2_2);

    ms5611_CompensateTemperature(&ms_primary);
}


// --- aux baro --
void mems_phase2_2_1()
{
    ms5611_ReadTemperature(&ms_aux, mems_phase2_2_2);
}

void mems_phase2_2_2()
{
    ms5611_StartPressure(&ms_aux, mems_phase2_2);
    ms5611_CompensateTemperature(&ms_aux);
}
// ------------

void mems_phase2_2()    //start pressure measurement command send
{
	if (fc.imu.status != fc_dev_error)
	{
	    //start reading fifo (acc & gyro)
	    lsm_start_acc(mems_phase2_3);
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
    lsm_read_acc();
    //start reading magnetometer
    lsm_start_mag(mems_phase2_4);
}

void mems_phase2_4()    //mag data  were loaded to buffer
{
    //load data from transfer buffer
    lsm_read_mag();

    lsm_start_gyro(mems_phase2_5);
}

void mems_phase2_5()
{
    lsm_read_gyro();

    //notify mems loop that we have new data to process
    osThreadFlagsSet(thread_mems, 0x01);
}

