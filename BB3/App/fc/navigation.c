/*
 * navigation.cpp
 *
 *  Created on: 30.07.2021
 *      Author: tilmann@bubecks.de
 */

#include "navigation.h"

#include "fc.h"
#include "etc/gnss_calc.h"

#define FC_ODO_MAX_SPEED_DIFF	(3) 	//10.8km/h
#define FC_ODO_MIN_SPEED		(0.277) //1km/h

#define NO_LAT_DATA  ((int32_t)2147483647)

void navigation_init()
{
	fc.flight.odometer = 0;
}

/**
 * Regularly called to do navigation work.
 */
void navigation_step()
{
	static int32_t last_lat = NO_LAT_DATA;
	static int32_t last_lon;

	static uint32_t next_update = 0;

	if (fc.gnss.fix > 0 && fc.gnss.status == fc_dev_ready && next_update < HAL_GetTick())
	{
		next_update = HAL_GetTick() + 1000;

		// Do we already have a previous GPS point?
		if (last_lat != NO_LAT_DATA)
		{
			bool use_fai = config_get_select(&config.units.earth_model) == EARTH_FAI;
			uint32_t v = gnss_distance(last_lat, last_lon, fc.gnss.latitude, fc.gnss.longtitude, use_fai, NULL);

			//do not add when gps speed is < 1 km/h
			//do not add when difference between calculated speed and gps speed is > 10 km/h
			if (fabs(v - fc.gnss.ground_speed) < FC_ODO_MAX_SPEED_DIFF && fc.gnss.ground_speed > FC_ODO_MIN_SPEED)
				fc.flight.odometer += v;
		}

		// Save the current GPS position for the next step
		last_lat = fc.gnss.latitude;
		last_lon = fc.gnss.longtitude;
	}
}
