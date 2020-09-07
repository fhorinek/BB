/*
 * gnss_sim33ela.cc
 *
 *  Created on: 4. 5. 2020
 *      Author: horinek
 */
#include "gnss_sim33ela.h"

#include "../debug.h"
#include "../fc/fc.h"
#include "../etc/epoch.h"
#include "../config/config.h"

//Pins
//#define	GPS_SW_EN
//#define	GPS_PPS
//#define	GPS_RESET

//DMA buffer
#define GNSS_BUFFER_SIZE	512
 uint8_t gnss_rx_buffer[GNSS_BUFFER_SIZE];

void sim33ela_init()
{
	fc.gnss.valid = false;
	fc.gnss.first_fix = true;

	HAL_UART_Receive_DMA(&gnss_uart, gnss_rx_buffer, GNSS_BUFFER_SIZE);

	GpioWrite(GPS_RESET, LOW);
	GpioWrite(GPS_SW_EN, HIGH);
	osDelay(10);
	GpioWrite(GPS_RESET, HIGH);
	fc.gnss.fix_time = HAL_GetTick();
}

void sim33ela_deinit()
{
	GpioWrite(GPS_SW_EN, LOW);
}

static void gnss_set_baudrate(uint32_t baud)
{
	gnss_uart.Init.BaudRate = baud;
	if (HAL_UART_Init(&gnss_uart) != HAL_OK)
	{
	  Error_Handler();
	}
}

static void nmea_send(const char * msg)
{
	uint8_t chsum = 0;

	for (uint8_t i = 0; i < strlen(msg); i++)
		chsum ^= msg[i];

	char nmea[128];
	sprintf(nmea, "$%s*%02X\r\n", msg, chsum);

//	DBG(">>> %s", nmea);
	HAL_UART_Transmit(&gnss_uart, (uint8_t *)nmea, strlen(nmea), 100);
}

static void nmea_start_configuration()
{
	INFO("Starting configuration");
	//GGA + RMC 10Hz
	//GGA + GSA 2Hz
	nmea_send("PMTK314,0,1,0,1,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0");
	//moule might be confused after baudrate change, send command twice
	nmea_send("PMTK314,0,1,0,1,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0");
}

static void nmea_parse_pmtk(char * buffer)
{
//	DBG("PMTK:%s", buffer);
	//Startup
	if (start_with(buffer, "010,001"))
	{
		//set baudrate
		nmea_send("PMTK251,921600");
	}
	else if (start_with(buffer, "001,314,3"))
	{
		//enable GPS + GLONAS + GALILEO
		nmea_send("PMTK353,1,1,1");

	}
	else if (start_with(buffer, "001,353,3"))
	{
		//set 10Hz
		nmea_send("PMTK220,100");
	}
	else if (start_with(buffer, "001,220,3"))
	{
		INFO("GNSS configured properly!");
	}
}

static void nmea_parse_rmc(char * buffer)
{
//	DBG("RMC:%s", buffer);

	char * ptr = find_comma(buffer);
	char * old_ptr;
	uint8_t tlen;

	//UTC time
	uint8_t hour = atoi_n(ptr + 0, 2);
	uint8_t min = atoi_n(ptr + 2, 2);
	uint8_t sec = atoi_n(ptr + 4, 2);

//	DEBUG("%02d:%02d:%02d\n", hour, min, sec);

	old_ptr = ptr;
	ptr = find_comma(ptr);
	tlen = ptr - old_ptr - 1;

	if (tlen != 10)
	{
		ERR("RMC bad timestamp len: %u", tlen);
		return;
	}

	if (*ptr == 'V')
	{
		return;
	}

	ptr = find_comma(ptr);

	uint32_t loc_deg;
	uint32_t loc_min;

	//Latitude, e.g. 4843.4437
	loc_deg = atoi_n(ptr, 2);        // 48
	loc_min = atoi_n(ptr + 2, 6);    // 434437000

	int32_t latitude = (loc_min * 100ul) / 6;
	latitude = loc_deg * GNSS_MUL + latitude;

	// DEBUG("lat: loc_deg=%ld loc_min=%ld, tlen=%d\n", loc_deg, loc_min, tlen);

	old_ptr = ptr;
	ptr = find_comma(ptr);
	tlen = ptr - old_ptr - 1;

	if (tlen != 9)
	{
		ERR("RMC bad latitude len: %u", tlen);
		return;
	}

	//Latitude sign
	if ((*ptr) == 'S')
		latitude *= -1;

	ptr = find_comma(ptr);

	//Longitude, 00909.2085
	loc_deg = atoi_n(ptr, 3);          // 009
	loc_min = atoi_n(ptr + 3, 6);      // 092085000

	int32_t longitude = (loc_min * 100ul) / 6;
	longitude = loc_deg * GNSS_MUL + longitude;

	// DEBUG("lon: loc_deg=%ld loc_min=%ld, tlen=%d\n", loc_deg, loc_min, tlen);

	old_ptr = ptr;
	ptr = find_comma(ptr);
	tlen = ptr - old_ptr - 1;


	if (tlen != 10)
	{
		ERR("RMC bad longitude len: %u", tlen);
		return;
	}

	//Longitude sign
	if ((*ptr) == 'W')
		longitude *= -1;

	ptr = find_comma(ptr);

	float gspd = atoi_f(ptr) * FC_KNOTS_TO_MPS; //in knots to m/s

	ptr = find_comma(ptr);

	//Ground course
	uint16_t hdg = (uint16_t)atoi_f(ptr);
	if (hdg >= 360)
	{
		ERR("RMC heading value: %u", hdg);
		return;
	}

	ptr = find_comma(ptr);

	//UTC date
	uint8_t day = atoi_n(ptr + 0, 2);
	uint8_t month = atoi_n(ptr + 2, 2);
	uint16_t year = atoi_n(ptr + 4, 2) + 2000;

	old_ptr = ptr;
	ptr = find_comma(ptr);
	tlen = ptr - old_ptr - 1;

	if (tlen != 6)
	{
		ERR("RMC bad date len: %u", tlen);
		return;
	}

	fc.gnss.latitude = latitude;
	fc.gnss.longtitude = longitude;
	fc.gnss.ground_speed = gspd;
	fc.gnss.heading = hdg;
	fc.gnss.utc_time = datetime_to_epoch(sec, min, hour, day, month, year);
}

static void nmea_parse_gga(char * buffer)
{
//	DBG("GGA:%s", buffer);

	char * ptr = find_comma(buffer);

	//Skip time
	ptr = find_comma(ptr);
	//Skip latitude
	ptr = find_comma(ptr);
	ptr = find_comma(ptr);
	//Skip longitude
	ptr = find_comma(ptr);
	ptr = find_comma(ptr);

	//Skip fix status
	ptr = find_comma(ptr);

	//skip Number of sat
	ptr = find_comma(ptr);

	//skip HDOP
	ptr = find_comma(ptr);

	//altitude
	fc.gnss.altitude = atoi_f(ptr);
	ptr = find_comma(ptr);

	//skip M
	ptr = find_comma(ptr);

	//Geoid
	fc.gnss.geoid_separation = atoi_f(ptr);
}

static void nmea_parse_gsa(uint8_t slot, char * buffer)
{
//	DBG("GSA[%u]: %s", slot, buffer);

	char * ptr = find_comma(buffer);

	//Skip mode
	ptr = find_comma(ptr);
	//fix status
	fc.gnss.sat_info[slot].fix = atoi_c(ptr);

	//update global fix status and gnss validity
	uint8_t best_fix = 0;
	if (config_get_bool(&config.devices.gnss.use_gps))
		best_fix = max(best_fix, fc.gnss.sat_info[GNSS_GPS].fix);
	if (config_get_bool(&config.devices.gnss.use_glonass))
		best_fix = max(best_fix, fc.gnss.sat_info[GNSS_GLONAS].fix);
	if (config_get_bool(&config.devices.gnss.use_galileo))
		best_fix = max(best_fix, fc.gnss.sat_info[GNSS_GALILEO].fix);

	fc.gnss.fix = best_fix;
	fc.gnss.valid = (best_fix > 1);

	if (fc.gnss.fix == 3 && fc.gnss.first_fix)
	{
		fc.gnss.fix_time = HAL_GetTick() - fc.gnss.fix_time;
		fc.gnss.first_fix = false;
	}

	ptr = find_comma(ptr);

	// Skip all 12 satellites
	for (uint8_t sat_no = 0; sat_no < 12; sat_no++ )
	{
		ptr = find_comma(ptr);
	}

	fc.gnss.sat_info[slot].pdop = atoi_f(ptr);
	ptr = find_comma(ptr);
	fc.gnss.sat_info[slot].hdop = atoi_f(ptr);
	ptr = find_comma(ptr);
	fc.gnss.sat_info[slot].vdop = atoi_f(ptr);
}

static void nmea_parse_gsv(uint8_t slot, char * buffer)
{
	DBG("GSV[%u]: %s", slot, buffer);

	char * ptr = find_comma(buffer);

	//Number of messages
	uint8_t msg_c = atoi_c(ptr);
	ptr = find_comma(ptr);
	//message number
	uint8_t msg_i = atoi_c(ptr);
	ptr = find_comma(ptr);
	//sat in view
	fc.gnss.sat_info[slot].sat_total = atoi_c(ptr);
	ptr = find_comma(ptr);

	for (uint8_t i = 0; i < 4 ; i++)
	{
		uint8_t sat_index = 4 * (msg_i - 1) + i;

		//sat_id
		fc.gnss.sat_info[slot].sats[sat_index].sat_id = atoi_c(ptr);
		ptr = find_comma(ptr);

		//skip elevation
		fc.gnss.sat_info[slot].sats[sat_index].elevation = atoi_c(ptr);
		ptr = find_comma(ptr);

		//skip azimut
		ptr = find_comma(ptr);
		fc.gnss.sat_info[slot].sats[sat_index].azimuth = atoi_c(ptr) / 2;

		//snr
		fc.gnss.sat_info[slot].sats[sat_index].snr = atoi_c(ptr);
		ptr = find_comma(ptr);
	}

	//if last message but not all sats, remove next sat info
	if (msg_i == msg_c && msg_i < 3)
	{
		for (uint8_t i = msg_i * 4; i < GNSS_NUMBER_OF_SATS; i++)
		{
			fc.gnss.sat_info[slot].sats[i].sat_id = 0;
		}
	}
}

static void nmea_parse(uint8_t c)
{
	#define FANET_IDLE		0
	#define FANET_DATA		1
	#define FANET_END		2
	#define NMEA_END		3

	#define NMEA_MAX_LEN	85

	static char parser_buffer[NMEA_MAX_LEN];
	static uint8_t parser_buffer_index;
	static uint8_t parser_checksum;
	static uint8_t parser_rx_checksum;
	static uint8_t parser_state = FANET_IDLE;

	switch (parser_state)
	{
		case(FANET_IDLE):
			if (c == '$')
			{
				parser_buffer_index = 0;
				parser_checksum = 0;
				parser_state = FANET_DATA;
			}
		break;

		case(FANET_DATA):
			if (c == '*')
			{
				parser_buffer[parser_buffer_index] = c;
				parser_buffer_index++;

				parser_state = FANET_END;
			}
			else
			{
				parser_checksum ^= c;
				parser_buffer[parser_buffer_index] = c;
				parser_buffer_index++;
			}

			if (parser_buffer_index >= NMEA_MAX_LEN)
			{
				ASSERT(0);
				parser_buffer_index = 0;
				parser_state = FANET_IDLE;
			}
		break;

		case(FANET_END):
			parser_rx_checksum = hex_to_num(c) << 4;
			parser_buffer[parser_buffer_index] = c;
			parser_buffer_index++;

			parser_state = NMEA_END;
		break;

		case(NMEA_END):
			parser_rx_checksum += hex_to_num(c);
			parser_buffer[parser_buffer_index] = c;

			parser_buffer[parser_buffer_index - 2] = '\0';

			parser_buffer_index = 0;
			parser_state = FANET_IDLE;

			if (parser_rx_checksum == parser_checksum)
			{
				//DBG("%s", parser_buffer);

				if (start_with(parser_buffer, "GNRMC"))
				{
					nmea_parse_rmc(parser_buffer + 5);
				}
				else if (start_with(parser_buffer, "GNGGA"))
				{
					nmea_parse_gga(parser_buffer + 5);
				}
				else if (start_with(parser_buffer + 2, "GSA"))
				{
					uint8_t slot;
					if (start_with(parser_buffer, "GP")) slot = GNSS_GPS;
					else if (start_with(parser_buffer, "GL")) slot = GNSS_GLONAS;
					else if (start_with(parser_buffer, "GA")) slot = GNSS_GALILEO;
					else
					{
						ERR("Unknown system %s", parser_buffer);
						return;
					}

					nmea_parse_gsa(slot, parser_buffer + 5);
				}
				else if (start_with(parser_buffer + 2, "GSV"))
				{
					uint8_t slot;
					if (start_with(parser_buffer, "GP")) slot = GNSS_GPS;
					else if (start_with(parser_buffer, "GL")) slot = GNSS_GLONAS;
					else if (start_with(parser_buffer, "GA")) slot = GNSS_GALILEO;
					else
					{
						ERR("Unknown system %s", parser_buffer);
						return;
					}

					nmea_parse_gsv(slot, parser_buffer + 5);
				}
				else if (start_with(parser_buffer, "PMTK"))
				{
					nmea_parse_pmtk(parser_buffer + 4);
				}
				else
					WARN("Not parsed \"%s\"", parser_buffer);
			}
			else
			{
				DBG("NMEA:\"$%s\"", parser_buffer);
				ERR("CHECKSUM ERROR! %02X %02X", parser_rx_checksum, parser_checksum);
			}
		break;
	}
}

void sim33ela_step()
{
	static uint16_t read_index = 0;
	static uint32_t last_data = 0;

	uint16_t write_index = GNSS_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(gnss_uart.hdmarx);
	uint16_t waiting;

	//Get number of bytes waiting in buffer
	if (read_index > write_index)
	{
		waiting = GNSS_BUFFER_SIZE - read_index + write_index;
	}
	else
	{
		waiting = write_index - read_index;
	}

	//unable to read the data for 2s -> set higher baudrate
	if (waiting == 0 && HAL_GetTick() - last_data > 2000)
	{
		gnss_set_baudrate(921600);
		nmea_start_configuration();
		last_data = HAL_GetTick();
	}

	if (waiting)
	{
		last_data = HAL_GetTick();
	}

	//parse the data
	for (uint16_t i = 0; i < waiting; i++)
	{
		nmea_parse(gnss_rx_buffer[read_index]);
		read_index = (read_index + 1) % GNSS_BUFFER_SIZE;
	}
}
