/*
 * gnss_calc.cc
 *
 *  Created on: May 29, 2020
 *      Author: horinek
 */

#include "gnss_calc.h"
#include "../fc/fc.h"

#define FAI_EARTH_RADIUS 6371

void get_kx_ky(float lat, float * kx, float * ky)
{
	float fcos = cos(to_radians(lat));
	float cos2 = 2. * fcos * fcos - 1.;
	float cos3 = 2. * fcos * cos2 - fcos;
	float cos4 = 2. * fcos * cos3 - cos2;
	float cos5 = 2. * fcos * cos4 - cos3;

    //multipliers for converting longitude and latitude
    //degrees into distance (http://1.usa.gov/1Wb1bv7)
	*kx = (111.41513 * fcos - 0.09455 * cos3 + 0.00012 * cos5);
    *ky = (111.13209 - 0.56605 * cos2 + 0.0012 * cos4);
}

void gnss_destination(float lat1, float lon1, float angle, float distance_km, float * lat2, float * lon2)
{
	angle = to_radians(angle);
	float dx = sin(angle) * distance_km;
	float dy = cos(angle) * distance_km;

	float kx, ky;
	get_kx_ky(lat1, &kx, &ky);

	*lon2 = lon1 + dx / kx;
	*lat2 = lat1 + dy / ky;
}

/**
 * Compute the distance between two GPS points in 2 dimensions
 * (without altitude). Latitude and longitude parameters must be given as fixed integers
 * multiplied with GPS_COORD_MULT.
 *
 * \param lat1 the latitude of the 1st GPS point
 * \param lon1 the longitude of the 1st GPS point
 * \param lat2 the latitude of the 2nd GPS point
 * \param lon2 the longitude of the 2nd GPS point
 * \param FAI use FAI sphere instead of WGS ellipsoid
 * \param bearing pointer to bearing (NULL if not used)
 *
 * \return the distance in m.
 */
uint32_t gnss_distance(int32_t lat1, int32_t lon1, int32_t lat2, int32_t lon2, bool FAI, int16_t * bearing)
{
//	DEBUG("*gps_distance\n");


	float d_lon = (lon2 - lon1) / (float)GNSS_MUL;
	float d_lat = (lat2 - lat1) / (float)GNSS_MUL;

//	DEBUG("#d_lon=%0.10f\n", d_lon);
//	DEBUG("#d_lat=%0.10f\n", d_lat);
//
//	DEBUG("lat1=%li\n", lat1);
//	DEBUG("lon1=%li\n", lon1);
//	DEBUG("lat2=%li\n", lat2);
//	DEBUG("lon2=%li\n", lon2);

	uint32_t dist;

	if (FAI)
	{
//		DEBUG("f=1\n");

		d_lon = to_radians(d_lon / 2);
		d_lat = to_radians(d_lat / 2);

//		DEBUG("#d_lon=%0.10f\n", d_lon);
//		DEBUG("#d_lat=%0.10f\n", d_lat);

		float q = pow(sin(d_lat), 2) + pow(sin(d_lon), 2) * cos(to_radians(lat1 / (float)GNSS_MUL)) * cos(to_radians(lat2 / (float)GNSS_MUL));

//		DEBUG("#q=%0.10f\n", q);

		dist = 2 * FAI_EARTH_RADIUS * asin(sqrt(q)) * 1000.0;
	}
	else //WGS
	{
//		DEBUG("f=0\n");
//		DEBUG("#lat=%0.10f\n", (lat1 + lat2) / ((float)GPS_COORD_MUL * 2));

		float kx, ky;
		get_kx_ky((lat1 + lat2) / ((float)GNSS_MUL * 2), &kx, &ky);

//		DEBUG("#kx=%0.10f\n", kx);
//		DEBUG("#ky=%0.10f\n", ky);

        d_lon *= kx;
        d_lat *= ky;

//		DEBUG("#d_lon=%0.10f\n", d_lon);
//		DEBUG("#d_lat=%0.10f\n", d_lat);

        dist = sqrt(pow(d_lon, 2) + pow(d_lat, 2)) * 1000.0;
	}

    if (bearing || 1)
    {
        if (d_lon  == 0 and d_lat == 0)
			*bearing = 0;

		*bearing = ((int16_t)to_degrees(atan2(d_lon, d_lat)) + 360) % 360;
//		DEBUG("a=%d\n", *bearing);
    }
//	DEBUG("d=%lu\n\n", dist);

    return dist;
}
