/*
 * gui.cc
 *
 *  Created on: Apr 23, 2020
 *      Author: horinek
 */
//#define DEBUG_LEVEL DBG_DEBUG

#include "gnss_thread.h"

#include "fanet.h"
#include "gnss_ublox_m8.h"

#include "fc/neighbors.h"
#include "fc/navigation.h"

/*
 * GPS simulation: This driver offers the functionality to simulate
 * GPS data read from a IGC file instead of the sensor. This allows
 * various flight simulations and checking, that the widgets (or other
 * code) behaves as expected.
 *
 * To use that feature define the preprocessor constant GPS_SIMULATION
 * and re-compile the code. Then place a IGC file onto the SD card as
 * "gps-sim.igc" which contains the track.
 */
// #define GPS_SIMULATION

#ifdef GPS_SIMULATION

// How many times should be simulation be faster than reality?
#define TIMEWARP 3

#include "fc/logger/igc.h"

static int32_t next_gps_sim = 0;
static int32_t fp_sim = 0;
static int heading = 0;

void gps_simulation()
{
	int32_t now = HAL_GetTick();
	bool ret;

	if ( now > next_gps_sim )
	{
		next_gps_sim = now + 1000;

		if ( fp_sim == 0 )
		{
			fp_sim = red_open("gps-sim.igc", RED_O_RDONLY);
			if (fp_sim < 0) {
				fp_sim = 0;
				return;
			}
		}
		flight_pos_t pos;
		for ( int i = 0; i < TIMEWARP; i++ )
			ret = igc_read_next_pos(fp_sim, &pos);
		if( ret )
		{
			//fc.gnss.itow = ubx_nav_posllh->iTOW;
			//disp_lat=  485547480; disp_lon =  93919890;   // Hohenneuffen
			fc.gnss.longtitude = pos.lon;
			fc.gnss.latitude = pos.lat;
			//fc.gnss.altitude_above_ellipsiod = ubx_nav_posllh->height / 1000.0;
			fc.gnss.altitude_above_msl= pos.gnss_alt;
			//fc.gnss.horizontal_accuracy = ubx_nav_posllh->hAcc / 100;
			//fc.gnss.vertical_accuracy = ubx_nav_posllh->vAcc / 100;
			fc.gnss.ground_speed = 10;

			fc.gnss.heading = heading;
			heading += 30;
			heading = heading % 360;

			fc.gnss.new_sample = 0xFF;
			fc.gnss.fix = 3;
		}
	}
}
#endif

uint32_t gnss_next_pps = 0;

void gnss_uart_rx_irq_ht()
{
	osThreadFlagsSet(thread_gnss, 0x01);
	DBG("GNSS RX HT");
}

void gnss_uart_rx_irq_tc()
{
	osThreadFlagsSet(thread_gnss, 0x01);
	DBG("GNSS RX TC");
}

void gnss_uart_rx_irq_idle()
{
	osThreadFlagsSet(thread_gnss, 0x01);
	DBG("GNSS RX IDLE");
}

void thread_gnss_start(void * argument)
{
    UNUSED(argument);

    system_wait_for_handle(&thread_gnss);

	INFO("Started");

	ublox_init();

	fanet_init();

	while (!system_power_off)
	{
#ifdef GPS_SIMULATION
		gps_simulation();
#else
		ublox_step();
#endif

		fanet_step();

		neighbors_step();

		DBG(" ---- Sleep start ---- ");
		int ret = osThreadFlagsWait(0x01, osFlagsWaitAny, 10);
		DBG(" ---- Sleep end (%d) ---- ", ret);
		(void)ret;
	}

    INFO("Done");
    osThreadSuspend(thread_gnss);
}
