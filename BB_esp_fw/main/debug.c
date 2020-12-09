#include "drivers/uart.h"
#include "../main/debug.h"

#include "../main/drivers/uart.h"
#include "../main/protocol.h"

void debug_send(uint8_t type, const char *format, ...)
{
	va_list arp;
	uint8_t msg_buff[256];
	uint16_t length;

	msg_buff[0] = type;

	va_start(arp, format);
	length = vsnprintf((char *)msg_buff + 1, sizeof(msg_buff) - 1, format, arp);
	va_end(arp);

	protocol_send(PROTO_DEBUG, msg_buff, length + 1);
}

void debug_dump(uint8_t * data, uint16_t len)
{
	char tmp[8 * 3 + 3];
	for (uint16_t i = 0; i < len; i++)
	{
		sprintf(tmp + ((i % 8) * 3) + ((i % 8 > 3) ? 1 : 0), "%02X  ", data[i]);
		if (i % 8 == 7 || i + 1 == len)
		{
			tmp[strlen(tmp) - 2] = 0;
			debug_send(DBG_DEBUG, "[%s]", tmp);
		}
	}
}
