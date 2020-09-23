#include "ms5611.h"
#include "ll/dma_i2c.h"
#include "../fc/fc.h"

static struct {
	 uint16_t C1;
	 uint16_t C2;
	 uint16_t C3;
	 uint16_t C4;
	 uint16_t C5;
	 uint16_t C6;
} calibration;

static uint32_t raw_temperature;
static uint32_t raw_pressure;
static int32_t dT;
static int32_t temperature;



void ms5611_ReadPROM()
{
	calibration.C1 = dma_i2c_read16(&mems_i2c, MS5611_ADDR, MS5611_PROM + 0);
	calibration.C2 = dma_i2c_read16(&mems_i2c, MS5611_ADDR, MS5611_PROM + 2);
	calibration.C3 = dma_i2c_read16(&mems_i2c, MS5611_ADDR, MS5611_PROM + 4);
	calibration.C4 = dma_i2c_read16(&mems_i2c, MS5611_ADDR, MS5611_PROM + 6);
	calibration.C5 = dma_i2c_read16(&mems_i2c, MS5611_ADDR, MS5611_PROM + 8);
	calibration.C6 = dma_i2c_read16(&mems_i2c, MS5611_ADDR, MS5611_PROM + 10);

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
	dma_i2c_cmd8(&mems_i2c, MS5611_ADDR, MS5611_RESET);
}

void ms5611_Init()
{
	ms5611_Reset();
	osDelay(10);
	ms5611_ReadPROM();
}


void ms5611_StartPressure()
{
	dma_i2c_cmd8(&mems_i2c, MS5611_ADDR, MS5611_D1 | MS5611_PRESS_OSR);
}

void ms5611_StartTemperature()
{
	dma_i2c_cmd8(&mems_i2c, MS5611_ADDR, MS5611_D2 | MS5611_TEMP_OSR);
}

void ms5611_ReadPressure()
{
    raw_pressure = dma_i2c_read24(&mems_i2c, MS5611_ADDR, MS5611_READ);
    ASSERT(raw_pressure != 0);
}

void ms5611_ReadTemperature()
{
	raw_temperature = dma_i2c_read24(&mems_i2c, MS5611_ADDR, MS5611_READ);
	ASSERT(raw_temperature != 0);
}

void ms5611_CompensateTemperature()
{
	dT = raw_temperature - (calibration.C5 * (int32_t)256);
	temperature = (2000ul + ((int64_t)dT * (int64_t)calibration.C6) / (int64_t)8388608);
}

float ms5611_CompensatePressure()
{
	int64_t off = (int64_t)calibration.C2 * (int64_t)65536 + ((int64_t)calibration.C4 * (int64_t)dT) / 128;
	int64_t sens = (int64_t)calibration.C1 * (int64_t)32768 + ((int64_t)calibration.C3 * (int64_t)dT) / 256;

	if (temperature < 2000)
	{
		//low temperature
		uint64_t t2 = (uint64_t)dT * (uint64_t)dT / 2147483648ul;
		uint64_t temp = (temperature - 2000) * (temperature - 2000);
		uint64_t off2 = 5 * temp / 2;
		uint64_t sens2 = off2 / 2;

		if (temperature < -1500)
		{
			//very low temperature
			temp = (temperature + 1500) * (temperature + 1500);
			off2 = off2 + 7 * temp;
			sens2 = sens2 + 11 * temp / 2;
		}

		temperature -= t2;
		off -= off2;
		sens -= sens2;
	}

	return (float)((int32_t)raw_pressure * sens / (int32_t)2097152 - off) / 32768.0;
}

float GetAltitude(float currentSeaLevelPressureInPa, float pressure)
{
    // Calculate altitude from sea level.
    float altitude = 44330.0 * (1.0 - pow(pressure / currentSeaLevelPressureInPa, 0.1902949571836346));
    return altitude;
}
