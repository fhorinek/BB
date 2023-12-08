/*
 * protocol.c
 *
 *  Created on: Oct 4, 2021
 *      Author: horinek
 */

#include "telemetry.h"

#include "drivers/esp/esp.h"
#include "drivers/esp/protocol.h"

#include "lk8ex1.h"
#include "openvario.h"
#include "gnss.h"
#include "fanet_forwarding.h"
#include "fanet.h"

#include "fc/fc.h"

#define PROTOCOL_PERIOD 100 //in ms - 10Hz

static osTimerId_t telemetry_timer;



//timer_cb
void telemetry_cb()
{
	proto_tele_send_t data;
	bool xmit = false;

	if (fc.esp.mode != esp_normal)
		osTimerStop(telemetry_timer);

	switch (config_get_select(&profile.bluetooth.protocol))
	{
		case(tele_lk8ex1):
			xmit = lk8ex1_msg(data.message, sizeof(data.message));
		break;

		case(tele_openvario):
			xmit = openvario_msg(data.message, sizeof(data.message));
		break;

		default:
		break;
	}

	if (xmit)
	{
		data.len = strlen(data.message);
		protocol_send(PROTO_TELE_SEND, (void *)&data, sizeof(data));
	}

	if (config_get_bool(&profile.bluetooth.forward_gnss))
	{

		if (fc.gnss.new_sample & FC_GNSS_NEW_SAMPLE_TELEMETRY)
		{
			fc.gnss.new_sample &= ~FC_GNSS_NEW_SAMPLE_TELEMETRY;

			if (gnss_rmc_msg(data.message, sizeof(data.message)))
			{
				INFO(">>%s<<", data.message);
				data.len = strlen(data.message);
				protocol_send(PROTO_TELE_SEND, (void *)&data, sizeof(data));
			}

			if (gnss_gga_msg(data.message, sizeof(data.message)))
			{
				INFO(">>%s<<", data.message);
				data.len = strlen(data.message);
				protocol_send(PROTO_TELE_SEND, (void *)&data, sizeof(data));
			}
		}
	}

	if (config_get_bool(&profile.bluetooth.forward_fanet)) {
		if(fanet_forwarding_msg(data.message, sizeof(data.message))) {
			data.len = strlen(data.message);
			INFO(">>%s<<", data.message);
			protocol_send(PROTO_TELE_SEND, (void *)&data, sizeof(data));
		}
	}

    if (config_get_bool(&profile.bluetooth.forward_fanet))
    {
        if (fanet_msg(data.message, sizeof(data.message)))
        {
            INFO(">>%s<<", data.message);
            data.len = strlen(data.message);
            protocol_send(PROTO_TELE_SEND, (void *)&data, sizeof(data));
        }
    }
}


//init
void telemetry_init()
{
	telemetry_timer = osTimerNew(telemetry_cb, osTimerPeriodic, NULL, NULL);
}

//start
void telemetry_start()
{
	if (config_get_bool(&profile.bluetooth.spp) || config_get_bool(&profile.bluetooth.ble))
		osTimerStart(telemetry_timer, PROTOCOL_PERIOD);
}

//stop
void telemetry_stop()
{
	osTimerStop(telemetry_timer);
}
