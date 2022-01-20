#define DEBUG_LEVEL DBG_DEBUG

#include "fanet.h"

#include "fc/fc.h"
#include "etc/epoch.h"
#include "fc/neighbors.h"
#include "etc/fanet_update.h"

//DMA buffer
#define FANET_BUFFER_SIZE	512
static uint8_t fanet_rx_buffer[FANET_BUFFER_SIZE];

#define FANET_BL_RESET	        0 //module is after reset
#define FANET_BL_CONTINUE       1 //module asked to bood
#define FANET_BL_DONE           2 //module booted


static uint8_t fanet_bootloader_state;
#define FANET_INIT_TIMEOUT 1500

static uint32_t fanet_start_time = 0;
static uint32_t fanet_next_transmit_tracking = 0;
static uint32_t fanet_next_transmit_name = 0;
static uint16_t fanet_read_index = 0;
bool fanet_need_update = false;
osSemaphoreId fanet_tx_lock = NULL;

void fanet_start_dma()
{
	//WARN("FANET Uart error");
	HAL_UART_Receive_DMA(fanet_uart, fanet_rx_buffer, FANET_BUFFER_SIZE);
	fanet_read_index = 0;
}


void fanet_reinit_uart()
{
    if (fanet_uart->Instance != NULL)
        HAL_UART_DeInit(fanet_uart);

    MX_UART8_Init();
    HAL_UART_Receive_DMA(fanet_uart, fanet_rx_buffer, FANET_BUFFER_SIZE);
    fanet_read_index = 0;
}

void fanet_tx_done()
{
    osSemaphoreRelease(fanet_tx_lock);
}

void fanet_init()
{
    if (fanet_tx_lock == NULL)
    {
        fanet_tx_lock = osSemaphoreNew(1, 0, NULL);
        osSemaphoreRelease(fanet_tx_lock);
    }

    fanet_reinit_uart();

	if (config_get_bool(&profile.fanet.enabled))
	{
		fanet_enable();
	}
	else
	{
	    fc.fanet.status = fc_dev_off;
	}
}

void fanet_enable()
{
	fanet_bootloader_state = FANET_BL_RESET;
    fc.fanet.status = fc_dev_init;
    fc.fanet.flags = 0;
    fanet_start_time = HAL_GetTick();
	neighbors_reset();

	GpioWrite(FANET_RST, LOW);
	GpioWrite(FANET_SW, HIGH);
	osDelay(10);
	GpioWrite(FANET_RST, HIGH);

    fanet_next_transmit_tracking = 0;
    fanet_next_transmit_name = 0;
}

void fanet_disable()
{
	neighbors_reset();
	fc.fanet.status = fc_dev_off;
	GpioWrite(FANET_SW, LOW);
	HAL_UART_DeInit(fanet_uart);
}

void fanet_send(const char * msg)
{
	char fmsg[128];

	DBG("<<< %s", msg);
	sprintf(fmsg, "#%s\n", msg);

	HAL_UART_Transmit(fanet_uart, (uint8_t *)fmsg, strlen(fmsg), 100);
}

void fanet_transmit(uint8_t * data, uint16_t len)
{
    if (osSemaphoreAcquire(fanet_tx_lock, 200) == osErrorTimeout)
        osSemaphoreRelease(fanet_tx_lock);

    uint8_t res = HAL_UART_Transmit_DMA(fanet_uart, data, len);
    ASSERT(res == HAL_OK);
}

uint16_t fanet_get_waiting()
{
    uint16_t write_index = FANET_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(fanet_uart->hdmarx);

    //Get number of bytes waiting in buffer
    if (fanet_read_index > write_index)
    {
        return FANET_BUFFER_SIZE - fanet_read_index + write_index;
    }
    else
    {
        return write_index - fanet_read_index;
    }
}

uint8_t fanet_read_byte()
{
    uint8_t byte = fanet_rx_buffer[fanet_read_index];
    fanet_read_index = (fanet_read_index + 1) % FANET_BUFFER_SIZE;
    return byte;
}

uint8_t fanet_peak_byte()
{
    uint8_t byte = fanet_rx_buffer[fanet_read_index];
    return byte;
}


void fanet_configure_flarm(bool init)
{
	if (!config_get_bool(&profile.fanet.enabled))
		return;

	//FANET enabled but not ready
	if (fc.fanet.status != fc_dev_ready && !init)
	{
		fc.fanet.flags |= FANET_FLARM_CHANGE_REG;
		return;
	}

	INFO("FANET Set Flarm");
	char cmd[8];
	snprintf(cmd, sizeof(cmd), "FAP %u", config_get_bool(&profile.fanet.flarm));
	fanet_send(cmd);
}

void fanet_configure_type(bool init)
{
	if (!config_get_bool(&profile.fanet.enabled))
		return;

	//FANET enabled but not ready
	if (fc.fanet.status != fc_dev_ready && !init)
	{
		fc.fanet.flags |= FANET_TYPE_CHANGE_REG;
		return;
	}

	uint8_t air = config_get_select(&profile.fanet.air_type);
	uint8_t track = config_get_select(&pilot.online_track);
	uint8_t ground = config_get_select(&profile.fanet.ground_type);

	INFO("FANET Set type");
	char cmd[16];
	snprintf(cmd, sizeof(cmd), "FNC %u,%u,%u", air, track, ground);
	fanet_send(cmd);
}

void fanet_set_mode()
{
	if (!config_get_bool(&profile.fanet.enabled))
		return;

	if (fc.fanet.status != fc_dev_ready)
	{
		fc.fanet.flags |= FANET_MODE_CHANGE_REG;
		return;
	}

	bool air_mode = fc.flight.mode == flight_flight;

	INFO("FANET Set mode");
	char cmd[16];
	snprintf(cmd, sizeof(cmd), "FNM %u", air_mode ? 0 : 1);
	fanet_send(cmd);
}

bool fanet_is_actual_version(char * module)
{
    FIL f;
    if (f_open(&f, PATH_FANET_FW, FA_READ) == FR_OK)
    {
        //version start
        f_lseek(&f, 0x18);

        char version[13];
        UINT br;
        f_read(&f, version, 12, &br);
        f_close(&f);

        version[12] = 0;
        if (br == 12)
        {
            DBG("FANET FW file version is '%s'", version);
            DBG("FANET module version is '%s'", module);
            if (strcmp(module, version) < 0)
            {
                INFO("FANET FW in module is older than file!");
                return false;
            }
            else
            {
                INFO("FANET FW is up to date!");
                return true;
            }
        }
        else
        {
            WARN("FANET FW file version read failed!");
            return true;
        }

    }
    else
    {
        WARN("FANET FW file not found!");
    }

    return true;
}

void fanet_parse_dg(char * buffer)
{
	DBG("DG: %s", buffer);

	if (start_with(buffer, "V "))
	{
	    if (start_with(buffer + 2, "build-"))
	    {
	        strcpy(fc.fanet.version, buffer + 2 + 6);
	    }
	    else
	    {
	        strcpy(fc.fanet.version, buffer + 2);
	    }

        if (!fanet_is_actual_version(fc.fanet.version) || config_get_bool(&config.debug.fanet_update))
	    {
            config_set_bool(&config.debug.fanet_update, false);

	        fanet_need_update = true;
	        fc.fanet.status = fc_dev_init;

	        //reset module
	        fanet_enable();
	    }
		//get module address
	    else if (fc.fanet.status == fc_dev_init)
	    {
			fanet_send("FNA");
	    }
	}
	//RX enabled
	else if (start_with(buffer, "R OK"))
	{
	    fc.fanet.flags |= FANET_TYPE_CHANGE_REG | FANET_FLARM_CHANGE_REG | FANET_MODE_CHANGE_REG;
	    fc.fanet.status = fc_dev_ready;
	    INFO("FANET module ready");
	}
}

void fanet_parse_msg(fanet_addr_t source, uint8_t type, uint8_t len, uint8_t * data, bool broadcast, bool signature)
{
	switch (type)
	{
		case(FANET_MSG_TYPE_TRACKING):
		{
			multi_value tmp;
			
			//latitude
//			memcpy((void *)&tmp, data + 0, 3);
			tmp.u32 = 0;
			tmp.u8[0] = data[0];
			tmp.u8[1] = data[1];
			tmp.u8[2] = data[2];
			int32_t lat = (tmp.s32 / 93206.0) * GNSS_MUL;

			//longitude
//			memcpy((void *)&tmp, data + 3, 3);
			tmp.u32 = 0;
			tmp.u8[0] = data[3];
			tmp.u8[1] = data[4];
			tmp.u8[2] = data[5];
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
			int16_t climb = complement2_7bit(data[9] & 0b01111111);
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
				int16_t turn = complement2_7bit(data[11] & 0b01111111) * 25;
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

			//latitude
//			memcpy((void *)&tmp, data + 0, 3);
			tmp.u32 = 0;
			tmp.u8[0] = data[0];
			tmp.u8[1] = data[1];
			tmp.u8[2] = data[2];
			int32_t lat = (tmp.s32 / 93206.0) * GNSS_MUL;

			//longitude
//			memcpy((void *)&tmp, data + 3, 3);
			tmp.u32 = 0;
			tmp.u8[0] = data[3];
			tmp.u8[1] = data[4];
			tmp.u8[2] = data[5];
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
	    fc.fanet.status = fc_dev_init;
	    //get version
		fanet_send("DGV");
	}
	else if (start_with(buffer, "A "))
	{
		unsigned int manu_id, user_id;
		sscanf(buffer + 2, "%02X,%04X", &manu_id, &user_id);
		fc.fanet.addr.manufacturer_id = manu_id;
		fc.fanet.addr.user_id = user_id;

		//check FLARM expiration
		if (fc.fanet.status == fc_dev_init)
			fanet_send("FAX");
	}
	else if (start_with(buffer, "F "))
	{
	    //incoming message
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
	DBG("FA: %s", buffer);

	//Get FLARM expiration
	if (start_with(buffer, "X "))
	{
		unsigned int year, month, day;
		sscanf(buffer + 2, "%u,%u,%u", &year, &month, &day);
		fc.fanet.flarm_expires = datetime_to_epoch(0, 0, 0, day, month + 1, year + 1900);

        //enable RX
        if (fc.fanet.status == fc_dev_init)
            fanet_send("DGP 1");
	}
}



void fanet_parse(uint8_t c)
{
	#define FANET_IDLE		0
	#define FANET_DATA		1
	#define FANET_END		2

	#define FANET_MAX_LEN	128

	static char parser_buffer[FANET_MAX_LEN];
	static uint8_t parser_buffer_index;
	static uint8_t parser_state = FANET_IDLE;

//	DBG("fanet_parse %u %c %02X", parser_state, c, c);

	switch (parser_state)
	{
	case(FANET_IDLE):
		if (c == '#')
		{
			parser_buffer_index = 0;
			parser_state = FANET_DATA;
			fanet_bootloader_state = FANET_BL_DONE;
		}
		else if (c == 'C')
		{
		    //After reset
		    if (fanet_bootloader_state == FANET_BL_RESET)
		    {
                if (fanet_need_update)
                {
                    //update bootloade
                    fanet_update_firmware();
					fanet_enable();

                    fanet_need_update = false;

                    //restart start timer
                    fanet_start_time = HAL_GetTick();
                }
                else
                {
                    //continue
                    char msg[] = "\n";
                    HAL_UART_Transmit(fanet_uart, (uint8_t *)msg, strlen(msg), 100);
                    fanet_bootloader_state = FANET_BL_CONTINUE;
                    fanet_start_time = HAL_GetTick();
                }
		    }
		}


	break;

	case(FANET_DATA):
		if (c != '\n')
		{
			parser_buffer[parser_buffer_index] = c;
			parser_buffer_index++;

			//message too long!
			if (parser_buffer_index > FANET_MAX_LEN)
			{
			    parser_state = FANET_IDLE;
			}
		}
		else
		{
			parser_buffer[parser_buffer_index] = 0;
			DBG(">>> %s", parser_buffer);

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
	float alt = fc.gnss.altitude_above_msl;

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
	float climb = fc.fused.vario;
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
	if (!config_get_bool(&profile.fanet.enabled))
		return;

	uint16_t waiting = 0;

    if (fanet_uart->hdmarx->State == HAL_DMA_STATE_RESET)
    {
        WARN("FANET uart DMA in RESET");
        fc.fanet.status = fc_dev_error;
        fanet_init();
    }
    else
    {
        uint16_t write_index = FANET_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(fanet_uart->hdmarx);

        //Get number of bytes waiting in buffer
        if (fanet_read_index > write_index)
        {
            waiting = FANET_BUFFER_SIZE - fanet_read_index + write_index;
        }
        else
        {
            waiting = write_index - fanet_read_index;
        }

        //parse the data
        for (uint16_t i = 0; i < waiting; i++)
        {
            fanet_parse(fanet_rx_buffer[fanet_read_index]);
            fanet_read_index = (fanet_read_index + 1) % FANET_BUFFER_SIZE;
        }
    }

    if (fc.fanet.status == fc_dev_init)
    {
        if(HAL_GetTick() - fanet_start_time > FANET_INIT_TIMEOUT)
        {
            if (fanet_bootloader_state == FANET_BL_CONTINUE)
            {
                WARN("FANET still in bootloader");
                fc.fanet.status = fc_dev_error;
                fanet_enable();
                fanet_need_update = true;
            }

            if (fanet_bootloader_state == FANET_BL_DONE)
            {
                WARN("FANET still in init state");
                fc.fanet.status = fc_dev_error;
                fanet_init();
            }
        }


    }

    if (fc.fanet.status == fc_dev_ready)
	{
		if (fc.fanet.flags & FANET_TYPE_CHANGE_REG)
		{
			fc.fanet.flags &= ~FANET_TYPE_CHANGE_REG;
			fanet_configure_type(false);
			return;
		}

		if (fc.fanet.flags & FANET_FLARM_CHANGE_REG)
		{
			fc.fanet.flags &= ~FANET_FLARM_CHANGE_REG;
			fanet_configure_flarm(false);
			return;
		}

		if (fc.fanet.flags & FANET_MODE_CHANGE_REG)
		{
			fc.fanet.flags &= ~FANET_MODE_CHANGE_REG;
			fanet_set_mode();
			return;
		}

        //if enabled tracking etc...
        if (fanet_next_transmit_tracking <= HAL_GetTick() && fc.gnss.fix == 3)
        {
            fanet_next_transmit_tracking = HAL_GetTick() + FANET_TX_TRACKING_PERIOD;

            fanet_transmit_pos();
        }

        if (fanet_next_transmit_name <= HAL_GetTick() && config_get_bool(&pilot.broadcast_name))
        {
            fanet_next_transmit_name = HAL_GetTick() + FANET_TX_NAME_PERIOD;

            fanet_addr_t dest;
            dest.manufacturer_id = FANET_ADDR_MULTICAST;
            dest.user_id = FANET_ADDR_MULTICAST;

            char * name = config_get_text(&pilot.name);

            fanet_transmit_message(FANET_MSG_TYPE_NAME, dest, strlen(name), (uint8_t *)name, false, false);
        }
	}
}
