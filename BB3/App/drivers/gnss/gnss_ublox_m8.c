/*
 * gnss_ublox_m8.cc
 *
 *  Created on: 4. 5. 2020
 *      Author: horinek
 */
//#define DEBUG_LEVEL DBG_DEBUG
#include "gnss_ublox_m8.h"

#include "fc/fc.h"
#include "etc/epoch.h"
#include "etc/notifications.h"

//Pins
//#define	GPS_SW_EN
//#define	GPS_PPS
//#define	GPS_RESET

//DMA buffer
#define GNSS_BUFFER_SIZE	2048
static uint8_t gnss_rx_buffer[GNSS_BUFFER_SIZE];
static uint8_t ublox_last_command;
static uint32_t ublox_last_command_time;
static uint32_t ublox_start_time;
static uint16_t ublox_read_index = 0;

#define UBLOX_INIT_TIMEOUT 1500

void ublox_start_dma()
{
	//WARN("GNSS Uart error");
	HAL_UART_Receive_DMA(gnss_uart, gnss_rx_buffer, GNSS_BUFFER_SIZE);
}


static enum {
	PM_INIT,
	PM_IDLE,
	PM_SYNC,
	PM_CLASS,
	PM_ID,
	PM_LEN_LO,
	PM_LEN_HI,
	PM_PAYLOAD,
	PM_CK_A,
	PM_CK_B,
	PM_RESET
} ublox_parse_mode;

void ublox_deinit()
{
	HAL_UART_DeInit(gnss_uart);
	ublox_parse_mode = PM_RESET;
}

void ublox_init()
{
	DBG("Ublox init");
	FC_ATOMIC_ACCESS
	{
		fc.gnss.status = fc_dev_init;
		fc.gnss.time_synced = false;
		fc.gnss.fake = false;
	}

	if (ublox_parse_mode != PM_INIT)
	{
		ublox_deinit();
	}

	MX_UART5_Init();
	HAL_UART_Receive_DMA(gnss_uart, gnss_rx_buffer, GNSS_BUFFER_SIZE);
	ublox_read_index = 0;

	GpioWrite(GNSS_RST, LOW);
	osDelay(100);
	GpioWrite(GNSS_RST, HIGH);

	ublox_start_time = HAL_GetTick();
	ublox_last_command_time = false;
}

static void gnss_set_baudrate(uint32_t baud)
{
	gnss_uart->Init.BaudRate = baud;
	if (HAL_UART_Init(gnss_uart) != HAL_OK)
	{
	    bsod_msg("GNSS baudrate set failed");
	}
}

static void ublox_send(uint8_t class, uint8_t id, uint8_t len, uint8_t * payload)
{
	uint8_t * buff = (uint8_t *)malloc(len + 8);
	byte2 blen;

	//sync word
	buff[0] = 0xB5;
	buff[1] = 0x62;

	//class
	buff[2] = class;

	//id
	buff[3] = id;

	//length
	blen.uint16 = len;
	buff[4] = blen.uint8[0];
	buff[5] = blen.uint8[1];

	//payload
	simple_memcpy(buff + 6, payload, len);

	//checksum
	uint8_t ck_a = 0;
	uint8_t ck_b = 0;
	for (uint8_t i = 2; i < len + 6; i++)
	{
		ck_a += buff[i];
		ck_b += ck_a;
	}
	buff[len + 6] = ck_a;
	buff[len + 7] = ck_b;

	DBG("UBX >>> %02X, %02X len %u", class, id, len);
	DUMP(payload, len);
	DBG("");

	HAL_UART_Transmit(gnss_uart, buff, len + 8, 100);

	free(buff);
}

static void ublox_command(uint8_t command)
{
	ublox_last_command = command;
	ublox_last_command_time = HAL_GetTick();

	DBG("sending command %u", command);

	switch (command)
	{
		case(0):
		{
			struct {
				uint8_t portID;
				uint8_t reserved1;
				uint16_t txReady;
				uint32_t mode;
				uint32_t baudRate;
				uint16_t inProtoMask;
				uint16_t outProtoMask;
				uint16_t flags;
				uint16_t reserved2;
			} ubx_cfg_prt;

			ubx_cfg_prt.portID = 1;
			ubx_cfg_prt.reserved1 = 0;
			ubx_cfg_prt.txReady = 0;
			ubx_cfg_prt.mode = (0b00100011000000); //1 stop, No parity, 8Bit;
			ubx_cfg_prt.baudRate = 115200;
			ubx_cfg_prt.inProtoMask = 0x0001;
			ubx_cfg_prt.outProtoMask = 0x0001;
			ubx_cfg_prt.flags = 0;
			ubx_cfg_prt.reserved2 = 0;

			DBG("ubx_cfg_prt");
			ublox_send(0x06, 0x00, sizeof(ubx_cfg_prt), (uint8_t *)&ubx_cfg_prt);
			gnss_set_baudrate(115200);
			break;
		}

		case(1):
		{
			struct {
				uint16_t measRate;
				uint16_t navRate;
				uint16_t timeRef;
			} ubx_cfg_rate;

			ubx_cfg_rate.measRate = 1000;
			ubx_cfg_rate.navRate = 1;
			ubx_cfg_rate.timeRef = 0;

			DBG("ubx_cfg_rate");
			ublox_send(0x06, 0x08, sizeof(ubx_cfg_rate), (uint8_t *)&ubx_cfg_rate);
			break;
		}

		case(2):
		case(3):
		case(4):
		case(5):
		case(6):
		{
			struct {
				uint8_t msgClass;
				uint8_t msgID;
				uint8_t rate;
			} ubx_cfg_msg;

			//output sat info
			ubx_cfg_msg.msgClass = 0x01;
			ubx_cfg_msg.rate = 1;

			switch (command)
			{
			case(2):
				ubx_cfg_msg.msgID = 0x35; //sat info
			break;
			case(3):
				ubx_cfg_msg.msgID = 0x03; //status
			break;
			case(4):
				ubx_cfg_msg.msgID = 0x21; //timeutc
			break;
			case(5):
				ubx_cfg_msg.msgID = 0x02; //POSLLH
			break;
			case(6):
				ubx_cfg_msg.msgID = 0x12; //velned
			break;
			}

			DBG("ubx_cfg_msg");
			ublox_send(0x06, 0x01, sizeof(ubx_cfg_msg), (uint8_t *)&ubx_cfg_msg);
			break;
		}
	}


}

bool ublox_handle_nav(uint8_t msg_id, uint8_t * msg_payload, uint16_t msg_len)
{
	DBG("Handle NAV");

	if (msg_id == 0x02)
	{
		ASSERT(msg_len == 28);

		typedef struct {
			uint32_t iTOW;
			int32_t lon;
			int32_t lat;
			int32_t height; //above elipsoid [mm]
			int32_t hMSL;  //above mean sea level [mm]
			uint32_t hAcc; //horizontal accuracy estimate [mm]
			uint32_t vAcc; //vertical accuracy estimate [mm]
		} ubx_nav_posllh_t;

		ubx_nav_posllh_t * ubx_nav_posllh = (ubx_nav_posllh_t *)msg_payload;

		FC_ATOMIC_ACCESS
		{
			fc.gnss.itow = ubx_nav_posllh->iTOW;
			fc.gnss.longtitude = ubx_nav_posllh->lon;
			fc.gnss.latitude = ubx_nav_posllh->lat;
			fc.gnss.altitude_above_ellipsiod = ubx_nav_posllh->height / 1000.0;
			fc.gnss.altitude_above_msl= ubx_nav_posllh->hMSL / 1000.0;
			fc.gnss.horizontal_accuracy = ubx_nav_posllh->hAcc / 100;
			fc.gnss.vertical_accuracy = ubx_nav_posllh->vAcc / 100;
			fc.gnss.new_sample = 0xFF;
		}

		config_set_big_int(&profile.ui.last_lat, fc.gnss.latitude);
		config_set_big_int(&profile.ui.last_lon, fc.gnss.longtitude);

		return true;
	}

	if (msg_id == 0x12)
	{
		ASSERT(msg_len == 36);

		typedef struct {
			uint32_t iTOW;
			int32_t velN;
			int32_t velE;
			int32_t velD;
			uint32_t speed; //3D speed [cm/s]
			uint32_t gSpeed; //ground speed [cm/s]
			int32_t heading; //degrees //1*10^-5
			uint32_t sAcc;
			uint32_t cAcc;
		} ubx_nav_velned_t;

		ubx_nav_velned_t * ubx_nav_velned = (ubx_nav_velned_t *)msg_payload;

		FC_ATOMIC_ACCESS
		{
			fc.gnss.ground_speed = ubx_nav_velned->gSpeed / 100.0;
			fc.gnss.heading = ubx_nav_velned->heading / 100000;
		}

		return true;
	}

	if (msg_id == 0x03)
	{
		ASSERT(msg_len == 16);

		typedef struct {
			uint32_t iTOW;
			uint8_t gpsFix;
			uint8_t flags;
			uint8_t fixStat;
			uint8_t flags2;
			uint32_t ttff;
			uint32_t msss;
		} ubx_nav_status_t;

		ubx_nav_status_t * ubx_nav_status = (ubx_nav_status_t *)msg_payload;

		FC_ATOMIC_ACCESS
		{
			switch (ubx_nav_status->gpsFix)
			{
				case(2):
					fc.gnss.fix = 2;
					fc.gnss.ttf = HAL_GetTick() - ublox_start_time;
                    if (fc.gnss.fix == 0)
                    	notification_send(notify_gnss_fix);
				break;
				case(3):
					fc.gnss.fix = 3;
					fc.gnss.ttf = ubx_nav_status->ttff;
                    if (fc.gnss.fix == 0)
                    	notification_send(notify_gnss_fix);
				break;
				default:
					fc.gnss.fix = 0;
					fc.gnss.ttf = HAL_GetTick() - ublox_start_time;
				break;
			}

		}

		return true;
	}

	if (msg_id == 0x21)//UBX-NAV-TIMEUTC
	{
		ASSERT(msg_len == 20);

		typedef struct {
			uint32_t iTOW;
			uint32_t tAcc;
			int32_t nano;
			uint16_t year;
			uint8_t month;
			uint8_t day;
			uint8_t hour;
			uint8_t min;
			uint8_t sec;
			uint8_t valid;
		} ubx_nav_timeutc_t;

		ubx_nav_timeutc_t * ubx_nav_timeutc = (ubx_nav_timeutc_t *)msg_payload;

		if ((ubx_nav_timeutc->valid & 0b00000111) == 0b00000111)
		{
			fc.gnss.utc_time = datetime_to_epoch(ubx_nav_timeutc->sec, ubx_nav_timeutc->min, ubx_nav_timeutc->hour,
					ubx_nav_timeutc->day, ubx_nav_timeutc->month, ubx_nav_timeutc->year);

			DBG("DATE %u.%u.%u", ubx_nav_timeutc->day, ubx_nav_timeutc->month, ubx_nav_timeutc->year);
			DBG("TIME %02u:%02u.%02u", ubx_nav_timeutc->hour, ubx_nav_timeutc->min, ubx_nav_timeutc->sec);
			DBG("UTC %lu", fc.gnss.utc_time);

			if (config_get_bool(&config.time.sync_gnss) && !fc.gnss.time_synced)
			{
				fc.gnss.time_synced = true;

				fc_set_time_from_utc(fc.gnss.utc_time);
			}
		}

		return true;
	}

	if (msg_id == 0x35)
	{
		ASSERT((msg_len - 8) % 12 == 0);

		typedef struct {
			uint32_t iTOW;
			uint8_t version;
			uint8_t numSvs;
			uint8_t reserved1[2];
		} ubx_nav_sat_t;

		ubx_nav_sat_t * ubx_nav_sat = (ubx_nav_sat_t *)msg_payload;

		DBG("iTOW %lu", ubx_nav_sat->iTOW);
		DBG("numSvs %u", ubx_nav_sat->numSvs);

		ASSERT(ubx_nav_sat->version == 0x01);

		typedef struct {
			uint8_t gnssId;
			uint8_t svId;
			uint8_t cno;
			int8_t elev;
			int16_t azim;
			int16_t prRes;
			uint32_t flags;
		} ubx_nav_sat_info_t;


		FC_ATOMIC_ACCESS
		{
			uint8_t used = 0;
			for (uint8_t i = 0; i < ubx_nav_sat->numSvs; i++)
			{
				ubx_nav_sat_info_t * ubx_nav_sat_info = (ubx_nav_sat_info_t *)(msg_payload + 8 + 12 * i);
				fc.gnss.sat_info.sats[i].sat_id = ubx_nav_sat_info->svId;
				fc.gnss.sat_info.sats[i].flags = GNSS_SAT_SYSTEM_MASK & ubx_nav_sat_info->gnssId;
				fc.gnss.sat_info.sats[i].snr = ubx_nav_sat_info->cno;
				fc.gnss.sat_info.sats[i].elevation = ubx_nav_sat_info->elev;
				fc.gnss.sat_info.sats[i].azimuth = ubx_nav_sat_info->azim / 2;
				if (ubx_nav_sat_info->flags & (1 << 3))
				{
					fc.gnss.sat_info.sats[i].flags |= GNSS_SAT_USED;
					used++;
				}
			}

			for (uint8_t i = ubx_nav_sat->numSvs; i < GNSS_NUMBER_OF_SATS; i++)
			{
				fc.gnss.sat_info.sats[i].sat_id = 0;
			}

			fc.gnss.sat_info.sat_total = ubx_nav_sat->numSvs;
			fc.gnss.sat_info.sat_used = used;
		}

		return true;
	}

	return false;
}

bool ublox_handle_ack(uint8_t msg_id, uint8_t * msg_payload, uint16_t msg_len)
{
	static uint8_t ubx_cfg_msg_cnt;

	DBG("Handle ACK");
	ASSERT(msg_len == 2);
	ASSERT(msg_id == 0x01);

	ublox_last_command_time = false;

	if (msg_payload[0] == 0x06 && msg_payload[1] == 0x00) //ubx_cfg_prt
	{
		ublox_command(1);
		return true;
	}

	if (msg_payload[0] == 0x06 && msg_payload[1] == 0x08) //ubx_cfg_rate
	{
		ublox_command(2);
		ubx_cfg_msg_cnt = 0;
		return true;
	}

	if (msg_payload[0] == 0x06 && msg_payload[1] == 0x01) //ubx_cfg_msg
	{
		if (ubx_cfg_msg_cnt < 4)
			ublox_command(3 + ubx_cfg_msg_cnt);

		ubx_cfg_msg_cnt++;
		return true;
	}

	return false;
}

static char ublox_start_word[] = "$GNRMC";


void ublox_parse(uint8_t b)
{
	DBG(">> %c %u", b, b);

	static __align uint8_t msg_payload[1024];

    static uint16_t index = 0;
    static uint16_t msg_len;

	static uint8_t msg_class;
	static uint8_t msg_id;
	static uint8_t msg_ck_a;
	static uint8_t msg_ck_b;

	if (ublox_parse_mode > PM_SYNC && ublox_parse_mode < PM_CK_A)
	{
		msg_ck_a += b;
		msg_ck_b += msg_ck_a;
	}

	switch (ublox_parse_mode)
	{
		case(PM_RESET):
			index = 0;
			ublox_parse_mode = PM_INIT;
		break;

		case(PM_INIT):
			{

				if (b == ublox_start_word[index])
				{
					index++;
					if (index == strlen(ublox_start_word))
					{
						ublox_command(0);
						ublox_parse_mode = PM_IDLE;
					}
				}
				else
				{
					index = 0;
				}

				break;
			}

		case(PM_IDLE):
			if (b == 0xB5)
			{
				ublox_parse_mode = PM_SYNC;
			}
			break;

		case(PM_SYNC):
			if (b == 0x62)
			{
				ublox_parse_mode = PM_CLASS;
				msg_ck_a = 0;
				msg_ck_b = 0;
			}
			else
			{
				ublox_parse_mode = PM_IDLE;
			}
			break;

		case(PM_CLASS):
			msg_class = b;
			ublox_parse_mode = PM_ID;
			break;

		case(PM_ID):
			msg_id = b;
			ublox_parse_mode = PM_LEN_LO;
			break;

		case(PM_LEN_LO):
			msg_len = b;
			ublox_parse_mode = PM_LEN_HI;
			break;

		case(PM_LEN_HI):
			msg_len |= (b << 8);
			index = 0;
			if (msg_len > 0)
				ublox_parse_mode = PM_PAYLOAD;
			else
				ublox_parse_mode = PM_CK_A;

			if (msg_len > sizeof(msg_payload))
			{
				ERR("payload too long");
				ublox_parse_mode = PM_IDLE;
			}

			break;
			
		case(PM_PAYLOAD):
			msg_payload[index] = b;
			index++;
			if (index == msg_len)
				ublox_parse_mode = PM_CK_A;
			break;
			
		case(PM_CK_A):
			if (msg_ck_a == b)
			{
				ublox_parse_mode = PM_CK_B;
			}
			else
			{
				ERR("Checksum A failed");
				ublox_parse_mode = PM_IDLE;
			}
			break;
			
		case(PM_CK_B):
			if (msg_ck_b == b)
			{
				//execute
				bool parsed;

				fc.gnss.status = fc_dev_ready;

				switch (msg_class)
				{
					case(0x05): //UBX-ACK
						parsed = ublox_handle_ack(msg_id, msg_payload, msg_len);
					break;

					case(0x01): //UBX-NAV
						parsed = ublox_handle_nav(msg_id, msg_payload, msg_len);
					break;

					default:
						parsed = false;
				}

				if (!parsed)
				{
					WARN("Message not parsed");
					DBG("UBX <<< %02X, %02X len %u", msg_class, msg_id, msg_len);
					DUMP(msg_payload, msg_len);
					DBG("");
				}
			}
			else
			{
				ERR("Checksum B failed");
			}
			ublox_parse_mode = PM_IDLE;

			break;
	}
}

uint16_t ublox_gnss_waiting;

void ublox_step()
{

    if (gnss_uart->hdmarx->State == HAL_DMA_STATE_RESET)
    {
        WARN("GNSS uart DMA in RESET");
        fc.gnss.status = fc_dev_error;
        ublox_init();
    }
    else
    {
        uint16_t write_index = GNSS_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(gnss_uart->hdmarx);

        //Get number of bytes waiting in buffer
        if (ublox_read_index > write_index)
        {
            ublox_gnss_waiting = GNSS_BUFFER_SIZE - ublox_read_index + write_index;
        }
        else
        {
            ublox_gnss_waiting = write_index - ublox_read_index;
        }

        //parse the data
        if (fc.gnss.fake == false)
        {
            for (uint16_t i = 0; i < ublox_gnss_waiting; i++)
            {
                ublox_parse(gnss_rx_buffer[ublox_read_index]);
                ublox_read_index = (ublox_read_index + 1) % GNSS_BUFFER_SIZE;
            }
        }
    }

	if (fc.gnss.status == fc_dev_init && HAL_GetTick() - ublox_start_time > UBLOX_INIT_TIMEOUT)
	{
		WARN("GNSS still in init state");
		fc.gnss.status = fc_dev_error;
		ublox_init();
	}


}
