/*
 * gnss_calc.cc
 *
 *  Created on: May 29, 2020
 *      Author: horinek
 */

#include "etc/geo_calc.h"
#include "fc/fc.h"

#define FAI_EARTH_RADIUS 6371

static float lat_mult[] = {
	1.00001269263410,
	1.00016503034732,
	1.00062227564598,
	1.00138512587137,
	1.00245474614468,
	1.00383277371586,
	1.00552132409524,
	1.00752299900955,
	1.00984089623881,
	1.01247862140355,
	1.01544030178698,
	1.01873060229028,
	1.02235474363937,
	1.02631852297526,
	1.03062833698348,
	1.03529120773669,
	1.04031481145009,
	1.04570751037551,
	1.05147838808628,
	1.05763728844289,
	1.06419485855898,
	1.07116259613477,
	1.07855290156473,
	1.08637913528301,
	1.09465568086611,
	1.10339801447867,
	1.11262278132588,
	1.12234787986046,
	1.13259255459192,
	1.14337749845791,
	1.15472496584871,
	1.16665889752463,
	1.17920505883836,
	1.19239119287568,
	1.20624719035817,
	1.22080527842164,
	1.23610023069989,
	1.25216960150919,
	1.26905398736196,
	1.28679731954458,
	1.30544719209465,
	1.32505523022013,
	1.34567750504505,
	1.36737500157145,
	1.39021414794172,
	1.41426741553005,
	1.43961400112117,
	1.46634060453179,
	1.49454231757694,
	1.52432364338688,
	1.55579966888401,
	1.58909741791146,
	1.62435741829468,
	1.66173552331931,
	1.70140503710806,
	1.74355920469978,
	1.78841414194345,
	1.83621229854950,
	1.88722657098226,
	1.94176521201939,
	2.00017772298141,
};


static void get_kx_ky(float lat, float * kx, float * ky)
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

//get pixel coordinates from reference lat, lon in specified zoom
void geo_to_pix(int32_t lon, int32_t lat, uint8_t zoom, int32_t g_lon, int32_t g_lat, int16_t * x, int16_t * y)
{
	int32_t step_x;
	int32_t step_y;
	geo_get_steps(lat, zoom, &step_x, &step_y);

	//get bbox
	uint32_t map_w = MAP_W * step_x;
	uint32_t map_h = MAP_H * step_y;
	int32_t lon1 = lon - map_w / 2;
	int32_t lat1 = lat + map_h / 2;

    int32_t d_lat = lat1 - g_lat;
    int32_t d_lon = g_lon - lon1;

    *x = d_lon / step_x;
    *y = d_lat / step_y;
}

//get size in pixels for one 1 degree
void geo_get_steps(int32_t lat, uint8_t zoom, int32_t * step_x, int32_t * step_y)
{
	zoom += 1;
	*step_x = (zoom * GNSS_MUL) / MAP_DIV_CONST;
	uint8_t lat_i = min(61, abs(lat / GNSS_MUL));
	*step_y = (zoom * GNSS_MUL / lat_mult[lat_i]) / MAP_DIV_CONST;
}


void geo_get_topo_steps(int32_t lat, int32_t step_x, int32_t step_y, int16_t * step_x_m, int16_t * step_y_m)
{
	uint8_t lat_i = min(60, abs(lat / GNSS_MUL));
    *step_x_m = step_x * 111000 / GNSS_MUL / lat_mult[lat_i];
    *step_y_m = step_y * 111000 / GNSS_MUL;
}

void geo_destination(float lat1, float lon1, float angle, float distance_km, float * lat2, float * lon2)
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
uint32_t geo_distance(int32_t lat1, int32_t lon1, int32_t lat2, int32_t lon2, bool FAI, int16_t * bearing)
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

    if (bearing)
    {
        if (d_lon  == 0 && d_lat == 0)
			*bearing = 0;
	else
		*bearing = ((int16_t)to_degrees(atan2(d_lon, d_lat)) + 360) % 360;
//		DEBUG("a=%d\n", *bearing);
    }
//	DEBUG("d=%lu\n\n", dist);

    return dist;
}
