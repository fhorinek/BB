
#include "fc/fc.h"
#include "drivers/power/pwr_mng.h"

bool lk8ex1_msg(char * buff, uint16_t len)
{
	char tmp[83];

	if (fc.fused.status != fc_dev_ready)
		return false;

	sprintf(tmp, "LK8EX1,%0.0f,99999,%0.0f,99,%u,", fc.fused.pressure, (fc.fused.vario * 100.0), pwr.fuel_gauge.battery_percentage + 1000);
	snprintf(buff, len, "$%s*%02X\r\n", tmp, nmea_checksum(tmp));

	return true;
}
