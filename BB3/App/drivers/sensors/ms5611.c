
#define DEBUG_LEVEL    DEBUG_DBG

#include "ms5611.h"

#include "fc/fc.h"

//registers of the device
#define MS5611_D1       0x40
#define MS5611_D2       0x50
#define MS5611_RESET    0x1E
#define MS5611_READ     0x00
#define MS5611_PROM     0xA2 // by adding ints from 0 to 6 we can read all the prom configuration values.

// OSR (Over Sampling Ratio) constants
#define MS5611_OSR_256  0x00
#define MS5611_OSR_512  0x02
#define MS5611_OSR_1024 0x04
#define MS5611_OSR_2048 0x06
#define MS5611_OSR_4096 0x08

#define MS5611_RESET    0x1E


#define MS5611_PRESS_OSR    MS5611_OSR_4096
#define MS5611_TEMP_OSR     MS5611_OSR_256


void ms5611_ReadPROM(ms_sensor_data_t * ms)
{
    ms->calibration.C1 = SWAP_UINT16(mems_i2c_read16(ms->addr, MS5611_PROM + 0));
    ms->calibration.C2 = SWAP_UINT16(mems_i2c_read16(ms->addr, MS5611_PROM + 2));
    ms->calibration.C3 = SWAP_UINT16(mems_i2c_read16(ms->addr, MS5611_PROM + 4));
    ms->calibration.C4 = SWAP_UINT16(mems_i2c_read16(ms->addr, MS5611_PROM + 6));
    ms->calibration.C5 = SWAP_UINT16(mems_i2c_read16(ms->addr, MS5611_PROM + 8));
    ms->calibration.C6 = SWAP_UINT16(mems_i2c_read16(ms->addr, MS5611_PROM + 10));

	DBG("ms5611 calibration data for %02X", ms->addr);
	DBG(" C1 %u", ms->calibration.C1);
	DBG(" C2 %u", ms->calibration.C2);
	DBG(" C3 %u", ms->calibration.C3);
	DBG(" C4 %u", ms->calibration.C4);
	DBG(" C5 %u", ms->calibration.C5);
	DBG(" C6 %u", ms->calibration.C6);
}

void ms5611_Reset(ms_sensor_data_t * ms)
{
	mems_i2c_cmd8(ms->addr, MS5611_RESET);
}

bool ms5611_init(ms_sensor_data_t * ms)
{
    if (!mems_i2c_test_device(ms->addr))
    {
        ms->present = false;
        ERR("MS5611 %02X not responding", ms->addr);
        return false;
    }
    else
    {
        ms->present = true;
        ms5611_Reset(ms);
        osDelay(5);
        ms5611_ReadPROM(ms);

        return true;
    }
}


void ms5611_StartPressure(ms_sensor_data_t * ms, mems_i2c_cb_t cb)
{
    mems_i2c_cmd8_start(ms->addr, MS5611_D1 | MS5611_PRESS_OSR, cb);
}

void ms5611_StartTemperature(ms_sensor_data_t * ms, mems_i2c_cb_t cb)
{
    mems_i2c_cmd8_start(ms->addr, MS5611_D2 | MS5611_TEMP_OSR, cb);
}

void ms5611_ReadPressure(ms_sensor_data_t * ms, mems_i2c_cb_t cb)
{
    mems_i2c_read24_start(ms->addr, MS5611_READ, &ms->raw_pressure, cb);
}

void ms5611_ReadTemperature(ms_sensor_data_t * ms, mems_i2c_cb_t cb)
{
    mems_i2c_read24_start(ms->addr, MS5611_READ, &ms->raw_temperature, cb);
}

void ms5611_CompensateTemperature(ms_sensor_data_t * ms)
{
    ms->raw_temperature = SWAP_UINT24(ms->raw_temperature);

	ms->dT = ms->raw_temperature - (ms->calibration.C5 * (int32_t)256);
	ms->temperature = (2000ul + ((int64_t)ms->dT * (int64_t)ms->calibration.C6) / (int64_t)8388608);
}

float ms5611_CompensatePressure(ms_sensor_data_t * ms)
{
    ms->raw_pressure = SWAP_UINT24(ms->raw_pressure);

	int64_t off = (int64_t)ms->calibration.C2 * (int64_t)65536 + ((int64_t)ms->calibration.C4 * (int64_t)ms->dT) / 128;
	int64_t sens = (int64_t)ms->calibration.C1 * (int64_t)32768 + ((int64_t)ms->calibration.C3 * (int64_t)ms->dT) / 256;

	if (ms->temperature < 2000)
	{
		//low ms5611_temperature
		uint64_t t2 = (uint64_t)ms->dT * (uint64_t)ms->dT / 2147483648ul;
		uint64_t temp = (ms->temperature - 2000) * (ms->temperature - 2000);
		uint64_t off2 = 5 * temp / 2;
		uint64_t sens2 = off2 / 2;

		if (ms->temperature < -1500)
		{
			//very low ms5611_temperature
			temp = (ms->temperature + 1500) * (ms->temperature + 1500);
			off2 = off2 + 7 * temp;
			sens2 = sens2 + 11 * temp / 2;
		}

		ms->temperature -= t2;
		off -= off2;
		sens -= sens2;
	}

	return (float)((int32_t)ms->raw_pressure * sens / (int32_t)2097152 - off) / 32768.0;
}

float GetAltitude(float currentSeaLevelPressureInPa, float pressure)
{
    // Calculate altitude from sea level.
    float altitude = 44330.0 * (1.0 - pow(pressure / currentSeaLevelPressureInPa, 0.1902949571836346));
    return altitude;
}
