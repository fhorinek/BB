
#include "fc/fc.h"

bool openvario_msg(char * buff, uint16_t len)
{
	char tmp[83];

	if (fc.fused.status != fc_dev_ready)
		return false;

	sprintf(tmp, "POW,P,%0.2f,E,%0.2f", fc.fused.pressure / 100.0, fc.fused.vario);
	snprintf(buff, len, "$%s*%02X\r\n", tmp, nmea_checksum(tmp));

	return true;
}
