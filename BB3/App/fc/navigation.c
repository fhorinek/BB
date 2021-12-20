/*
 * navigation.cpp
 *
 *  Created on: 30.07.2021
 *      Author: tilmann@bubecks.de
 */

#include <etc/geo_calc.h>
#include "navigation.h"

#include "fc.h"

#define FC_ODO_MAX_SPEED_DIFF	(3) 	//10.8km/h
#define FC_ODO_MIN_SPEED		(0.277) //1km/h

#define NO_LAT_DATA  ((int32_t)2147483647)

void navigation_init()
{
	fc.flight.odometer = 0;
	fc.flight.toff_dist = 0;
	fc.flight.toff_bearing = 0;
}

/**
 * Regularly called to do navigation work.
 */
void navigation_step()
{
	static int32_t last_lat = NO_LAT_DATA;
	static int32_t last_lon;
	static uint32_t last_time;

	if (fc.gnss.fix > 0 && last_time != fc.gnss.itow)
	{
		// Do we already have a previous GPS point?
		if (last_lat != NO_LAT_DATA)
		{
			bool use_fai = config_get_select(&config.units.earth_model) == EARTH_FAI;
			uint32_t dist = geo_distance(last_lat, last_lon, fc.gnss.latitude, fc.gnss.longtitude, use_fai, NULL);

			uint32_t delta = fc.gnss.itow - last_time;
			float speed = dist * (1000.0 / delta);
			DBG("%lu %0.2f %lu", dist, speed, delta);

			//do not add when gps speed is < 1 km/h
			//do not add when difference between calculated speed and gps speed is > 10 km/h
			if (fabs(speed - fc.gnss.ground_speed) < FC_ODO_MAX_SPEED_DIFF && fc.gnss.ground_speed > FC_ODO_MIN_SPEED)
				fc.flight.odometer += dist;
		}

		// Save the current GPS position for the next step
		last_lat = fc.gnss.latitude;
		last_lon = fc.gnss.longtitude;
		last_time = fc.gnss.itow;

		navigation_toff_dist_bearing();
	}
}

/**
 * Calculate distance & bearing to take off
 */
void navigation_toff_dist_bearing()
{
	if (fc.flight.start_lat != 0) {
		bool use_fai = config_get_select(&config.units.earth_model) == EARTH_FAI;
		int16_t bearing = 0;
		uint32_t dist = geo_distance(fc.gnss.latitude, fc.gnss.longtitude, fc.flight.start_lat, fc.flight.start_lon, use_fai, &bearing);
		fc.flight.toff_dist = dist;
		fc.flight.toff_bearing = bearing;
	}
}
