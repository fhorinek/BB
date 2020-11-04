#include "fanet.h"

#include "../fc/fc.h"
#include "../etc/epoch.h"
#include "../config/config.h"
#include "../fc/neighbors.h"

//DMA buffer
#define FANET_BUFFER_SIZE	512
static uint8_t fanet_rx_buffer[FANET_BUFFER_SIZE];

#define FANET_BL_RESET	0
#define FANET_BL_OFF	1

static uint8_t fanet_bootloader_state;

void fanet_init()
{
	fc.fanet.valid = false;

	HAL_UART_Receive_DMA(&fanet_uart, fanet_rx_buffer, FANET_BUFFER_SIZE);
	neighbors_reset();

	if (config_get_bool(&config.devices.fanet.enabled))
	{
		fanet_enable();
	}
}

void fanet_enable()
{
	fanet_bootloader_state = FANET_BL_RESET;

	GpioWrite(FN_RST, LOW);
	GpioWrite(FN_EN, HIGH);
	osDelay(10);
	GpioWrite(FN_RST, HIGH);
}

void fanet_disable()
{
	neighbors_reset();
	fc.fanet.valid = false;
	GpioWrite(FN_EN, LOW);
}

void fanet_send(const char * msg)
{
	char fmsg[128];

	DBG(">>> %s", msg);
	sprintf(fmsg, "#%s\n", msg);

	HAL_UART_Transmit(&fanet_uart, (uint8_t *)fmsg, strlen(fmsg), 100);
}

void fanet_parse_dg(char * buffer)
{
	DBG("DG: %s", buffer);

	if (start_with(buffer, "V "))
	{
		strcpy(fc.fanet.version, buffer + 2);
		fanet_send("FNA");
	}
}

void fanet_parse_msg(fanet_addr_t source, uint8_t type, uint8_t len, uint8_t * data, bool broadcast, bool signature)
{
	switch (type)
	{
		case(FANET_MSG_TYPE_TRACKING):
		{
			multi_value tmp;
			tmp.u32 = 0;

			
			//latitude
			memcpy((void *)&tmp, data + 0, 3);
			int32_t lat = (tmp.s32 / 93206.0) * GNSS_MUL;

			//longitude
			memcpy((void *)&tmp, data + 3, 3);
			int32_t lon = (tmp.s32 / 46603.0) * GNSS_MUL;

			//altitude
			tmp.u32 = 0;
			tmp.u8[0] = data[6];
			tmp.u8[1] = data[7] & 0b00000111;

			uint16_t alt = tmp.u16[0];
			if (data[7] & 0b00001000)
				alt *=  4;

			//speed in km/h * 10
			uint16_t spd = (data[8] & 0b01111111) * 5;
			if (data[8] & 0b10000000)
				spd *= 5;

			//climb in m/s * 10
			int16_t climb = complement7(data[9] & 0b01111111);
			if (data[9] & 0b10000000)
				climb *= 5;

			//heading 360 == 255
			uint8_t hdg = data[10];

			//flags
			uint8_t flags = (NB_AIRCRAFT_TYPE_MASK & (data[7] >> 4)) | NB_IS_FLYING;
			if (data[7] & 0b10000000)
				flags |= NB_ONLINE_TRACKING;

			int16_t turn = 0;
			if (len > 10)
			{
				//turn rate in deg/s * 100
				int16_t turn = complement7(data[11] & 0b01111111) * 25;
				if (data[11] & 0b10000000)
					turn *= 5;

				flags |= NB_HAVE_TURNRATE;
			}

			neighbor_t nb;
			nb.addr = source;
			nb.latitude = lat;
			nb.longitude = lon;
			nb.heading = hdg;
			nb.flags = flags;
			nb.alititude = alt;

			neighbors_update(nb);

		}
		break;

		case(FANET_MSG_TYPE_NAME):
		{
			INFO("%02X:%04X : %s", source.manufacturer_id, source.user_id, data);
			neighbors_update_name(source, (char *)data);
		}
		break;

		case(FANET_MSG_TYPE_GROUND_TRACKING):
		{
			multi_value tmp;
			tmp.u32 = 0;

			//latitude
			memcpy((void *)&tmp, data + 0, 3);
			int32_t lat = (tmp.s32 / 93206.0) * GNSS_MUL;

			//longitude
			memcpy((void *)&tmp, data + 3, 3);
			int32_t lon = (tmp.s32 / 46603.0) * GNSS_MUL;

			//flags
			uint8_t flags = NB_GROUND_TYPE_MASK & (data[6] >> 4);
			if (data[6] & 0b00000001)
				flags |= NB_ONLINE_TRACKING;

			neighbor_t nb;
			nb.addr = source;
			nb.latitude = lat;
			nb.longitude = lon;
			nb.flags = flags;

			neighbors_update(nb);
		}
		break;
		
	}
}

void fanet_parse_fn(char * buffer)
{
	DBG("FN: %s", buffer);

	if (start_with(buffer, "R MSG,1,initialized"))
	{
		fanet_send("DGV");
	}
	else if (start_with(buffer, "A "))
	{
		unsigned int manu_id, user_id;
		sscanf(buffer + 2, "%02X,%04X", &manu_id, &user_id);
		fc.fanet.addr.manufacturer_id = manu_id;
		fc.fanet.addr.user_id = user_id;

		//TDOD: set air type. livetracking, grountype
		fanet_send("FNC 1,1,1");
	}
	else if (start_with(buffer, "R OK") && !fc.fanet.valid)
	{
		//enable RX
		fanet_send("DGP 1");
		fc.fanet.valid = true;
	}
	else if (start_with(buffer, "F "))
	{
		buffer = buffer + 2;

		//address
		fanet_addr_t source;
		source.manufacturer_id = atoi_hex32(buffer);
		buffer = find_comma(buffer);
		source.user_id = atoi_hex32(buffer);

		//broadcast
		buffer = find_comma(buffer);
		bool broadcast = *buffer == '1';

		//signature
		buffer = find_comma(buffer);
		bool signature = *buffer == '1';

		//type
		buffer = find_comma(buffer);
		uint8_t type = atoi_hex32(buffer);

		//length
		buffer = find_comma(buffer);
		uint8_t len = atoi_hex32(buffer);

		//data
		uint8_t data[256];
		buffer = find_comma(buffer);
		for (uint8_t i = 0; i < len; i++)
			data[i] = atoi_hex8(buffer + (i * 2));
		data[len] = 0;

		fanet_parse_msg(source, type, len, data, broadcast, signature);
	}

}

void fanet_parse_fa(char * buffer)
{
	//DBG("FA: %s", buffer);
}

void fanet_parse(uint8_t c)
{
	#define FANET_IDLE		0
	#define FANET_DATA		1
	#define FANET_END		2

	#define FANET_MAX_LEN	128

	static char parser_buffer[FANET_MAX_LEN];
	static uint8_t parser_buffer_index;
	uint8_t parser_state = FANET_IDLE;

	switch (parser_state)
	{
	case(FANET_IDLE):
		if (c == '#')
		{
			parser_buffer_index = 0;
			parser_state = FANET_DATA;
		}
		if (c == 'C' && fanet_bootloader_state == FANET_BL_RESET)
		{
			char msg[] = "\n";

			//if need to go to bootloader
			// ...
			//else
			HAL_UART_Transmit(&fanet_uart, (uint8_t *)msg, strlen(msg), 100);
			fanet_bootloader_state = FANET_BL_OFF;
		}
	break;

	case(FANET_DATA):
			break;
		if (c != '\n')
		{
			parser_buffer[parser_buffer_index] = c;
			parser_buffer_index++;
		}
		else
		{
			parser_buffer[parser_buffer_index] = 0;

			if (start_with(parser_buffer, "FN"))
			{
				fanet_parse_fn(parser_buffer + 2);
			}
			else if (start_with(parser_buffer, "DG"))
			{
				fanet_parse_dg(parser_buffer + 2);
			}
			else if (start_with(parser_buffer, "FA"))
			{
				fanet_parse_fa(parser_buffer + 2);
			}
			else
				WARN("Not parsed %s", parser_buffer);

			parser_state = FANET_IDLE;
		}
	break;
	}
}

void fanet_transmit_pos()
{
	char line[80];

	float lat = fc.gnss.latitude / (float)GNSS_MUL;
	float lon = fc.gnss.longtitude / (float)GNSS_MUL;
	float alt = fc.gnss.altitude_above_ellipsiod;

	uint16_t year;
	uint8_t month;
	uint8_t wday;
	uint8_t day;
	uint8_t hour;
	uint8_t min;
	uint8_t sec;

	datetime_from_epoch(fc.gnss.utc_time, &sec, &min, &hour, &day, &wday, &month, &year);

	year -= 1900;
	month -= 1;

	float speed = 0;
	float climb = 0;
	int16_t hdg = fc.gnss.heading;
	float deg_s = 0;

	sprintf(line, "FNS %0.5f,%0.5f,%0.1f,%0.2f,%0.2f,%d,%u,%u,%u,%u,%u,%0.1f", lat, lon, alt, speed, climb, hdg, year, month, day, hour, sec, deg_s);
	fanet_send(line);
}

void fanet_transmit_message(uint8_t type, fanet_addr_t dest, uint8_t len, uint8_t * payload, bool forward, bool ack_required)
{
	char line[128];

	sprintf(line, "FNT %X,%X,%X,%X,%X,%X,", type, dest.manufacturer_id, dest.user_id, forward, ack_required, len);

	for (uint8_t i = 0; i < len; i++)
	{
		sprintf(line + strlen(line), "%02X", payload[i]);
	}

	fanet_send(line);
}

void fanet_step()
{
	if (!config_get_bool(&config.devices.fanet.enabled))
		return;

	static uint16_t read_index = 0;
	static uint32_t next_transmit_tracking = 0;
	static uint32_t next_transmit_name = 0;

	uint16_t write_index = FANET_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(fanet_uart.hdmarx);
	uint16_t waiting;

	//Get number of bytes waiting in buffer
	if (read_index > write_index)
	{
		waiting = FANET_BUFFER_SIZE - read_index + write_index;
	}
	else
	{
		waiting = write_index - read_index;
	}

	//parse the data
	for (uint16_t i = 0; i < waiting; i++)
	{
		fanet_parse(fanet_rx_buffer[read_index]);
		read_index = (read_index + 1) % FANET_BUFFER_SIZE;
	}

	//if enabled tracking etc...
	if (next_transmit_tracking <= HAL_GetTick() && fc.fanet.valid/* && fc.gnss.valid*/)
	{
		next_transmit_tracking = HAL_GetTick() + FANET_TX_TRACKING_PERIOD;

		fanet_transmit_pos();
	}

	if (next_transmit_name <= HAL_GetTick() && fc.fanet.valid && config_get_bool(&config.devices.fanet.broadcast_name))
	{
		next_transmit_name = HAL_GetTick() + FANET_TX_NAME_PERIOD;

		fanet_addr_t dest;
		dest.manufacturer_id = FANET_ADDR_MULTICAST;
		dest.user_id = FANET_ADDR_MULTICAST;

		char * name = config_get_text(&config.pilot.name);

		fanet_transmit_message(FANET_MSG_TYPE_NAME, dest, strlen(name), (uint8_t *)name, false, false);
	}
}
