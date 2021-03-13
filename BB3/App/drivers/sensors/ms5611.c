
#define DEBUG_LEVEL    DEBUG_DBG

#include "ms5611.h"

#include "fc/fc.h"

static struct {
	 uint16_t C1;
	 uint16_t C2;
	 uint16_t C3;
	 uint16_t C4;
	 uint16_t C5;
	 uint16_t C6;
} calibration;

volatile uint32_t ms5611_raw_temperature;
volatile uint32_t ms5611_raw_pressure;
int32_t ms5611_dT = 2366;
int32_t ms5611_temperature = 2007;

void ms5611_ReadPROM()
{
    calibration.C1 = SWAP_UINT16(mems_i2c_read16(MS5611_ADDR, MS5611_PROM + 0));
    calibration.C2 = SWAP_UINT16(mems_i2c_read16(MS5611_ADDR, MS5611_PROM + 2));
    calibration.C3 = SWAP_UINT16(mems_i2c_read16(MS5611_ADDR, MS5611_PROM + 4));
    calibration.C4 = SWAP_UINT16(mems_i2c_read16(MS5611_ADDR, MS5611_PROM + 6));
    calibration.C5 = SWAP_UINT16(mems_i2c_read16(MS5611_ADDR, MS5611_PROM + 8));
    calibration.C6 = SWAP_UINT16(mems_i2c_read16(MS5611_ADDR, MS5611_PROM + 10));

	DBG("ms5611 calibration data");
	DBG(" C1 %u", calibration.C1);
	DBG(" C2 %u", calibration.C2);
	DBG(" C3 %u", calibration.C3);
	DBG(" C4 %u", calibration.C4);
	DBG(" C5 %u", calibration.C5);
	DBG(" C6 %u", calibration.C6);
}



void ms5611_Reset()
{
	mems_i2c_cmd8(MS5611_ADDR, MS5611_RESET);
}

void ms5611_init()
{
    if (!mems_i2c_test_device(MS5611_ADDR))
    {
        fc.baro.status = fc_dev_error;
        ERR("MS5611 not responding");
    }
    else
    {
        ms5611_Reset();
        osDelay(5);
        ms5611_ReadPROM();

        fc.baro.status = fc_dev_ready;
    }
}


void ms5611_StartPressure(mems_i2c_cb_t cb)
{
    mems_i2c_cmd8_start(MS5611_ADDR, MS5611_D1 | MS5611_PRESS_OSR, cb);
}

void ms5611_StartTemperature(mems_i2c_cb_t cb)
{
    mems_i2c_cmd8_start(MS5611_ADDR, MS5611_D2 | MS5611_TEMP_OSR, cb);
}

void ms5611_ReadPressure(mems_i2c_cb_t cb)
{
    mems_i2c_read24_start(MS5611_ADDR, MS5611_READ, &ms5611_raw_pressure, cb);
}

void ms5611_ReadTemperature(mems_i2c_cb_t cb)
{
    mems_i2c_read24_start(MS5611_ADDR, MS5611_READ, &ms5611_raw_temperature, cb);
}

void ms5611_CompensateTemperature()
{
    ms5611_raw_temperature = SWAP_UINT24(ms5611_raw_temperature);

	ms5611_dT = ms5611_raw_temperature - (calibration.C5 * (int32_t)256);
	ms5611_temperature = (2000ul + ((int64_t)ms5611_dT * (int64_t)calibration.C6) / (int64_t)8388608);
}

float ms5611_CompensatePressure()
{
    ms5611_raw_pressure = SWAP_UINT24(ms5611_raw_pressure);

	int64_t off = (int64_t)calibration.C2 * (int64_t)65536 + ((int64_t)calibration.C4 * (int64_t)ms5611_dT) / 128;
	int64_t sens = (int64_t)calibration.C1 * (int64_t)32768 + ((int64_t)calibration.C3 * (int64_t)ms5611_dT) / 256;

	if (ms5611_temperature < 2000)
	{
		//low ms5611_temperature
		uint64_t t2 = (uint64_t)ms5611_dT * (uint64_t)ms5611_dT / 2147483648ul;
		uint64_t temp = (ms5611_temperature - 2000) * (ms5611_temperature - 2000);
		uint64_t off2 = 5 * temp / 2;
		uint64_t sens2 = off2 / 2;

		if (ms5611_temperature < -1500)
		{
			//very low ms5611_temperature
			temp = (ms5611_temperature + 1500) * (ms5611_temperature + 1500);
			off2 = off2 + 7 * temp;
			sens2 = sens2 + 11 * temp / 2;
		}

		ms5611_temperature -= t2;
		off -= off2;
		sens -= sens2;
	}

	return (float)((int32_t)ms5611_raw_pressure * sens / (int32_t)2097152 - off) / 32768.0;
}

float GetAltitude(float currentSeaLevelPressureInPa, float pressure)
{
    // Calculate altitude from sea level.
    float altitude = 44330.0 * (1.0 - pow(pressure / currentSeaLevelPressureInPa, 0.1902949571836346));
    return altitude;
}
