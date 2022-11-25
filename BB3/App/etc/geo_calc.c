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

// number of elements inside lat_mult array:
#define LAT_MULT_NUM_ELEMENTS (sizeof(lat_mult)/sizeof(float))

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
	geo_to_pix_w_h(lon, lat, zoom, g_lon, g_lat, x, y, MAP_W, MAP_H);
}

//get pixel coordinates from reference lat, lon in specified zoom
void geo_to_pix_w_h(int32_t lon, int32_t lat, uint8_t zoom, int32_t g_lon, int32_t g_lat, int16_t * x, int16_t * y, int16_t w, int16_t h)
{
	int32_t step_x;
	int32_t step_y;
	geo_get_steps(lat, zoom, &step_x, &step_y);

	//get bbox
	uint32_t map_w = w * step_x;
	uint32_t map_h = (h * step_y);
	int32_t lon1 = lon - map_w / 2;
	int32_t lat1 = lat + map_h / 2;

    int32_t d_lat = lat1 - g_lat;
    int32_t d_lon = g_lon - lon1;

    *x = d_lon / step_x;
    *y = d_lat / step_y;
}


void align_to_cache_grid(int32_t lon, int32_t lat, uint16_t zoom, int32_t * c_lon, int32_t * c_lat)
{
    int32_t step_x;
    int32_t step_y;

    uint16_t zoom_p = pow(2, min(zoom, 9));

    step_x = (zoom_p * (int64_t)GNSS_MUL) / MAP_DIV_CONST;

    //get bbox
    int32_t map_w = (MAP_W * step_x);
//    uint32_t map_h = (MAP_H * step_x);//0;
    int32_t map_h = 0;

    *c_lon = (lon / map_w) * map_w + map_w / 2;
//    *c_lat = (lat / map_h) * map_h + map_h / 2;
//    return;

    int32_t t_lat = 0;
    int8_t last_lat = -1;
    int32_t last_map_h = 0;

    while (t_lat + map_h < lat)
    {
        t_lat += map_h;

        if (t_lat / GNSS_MUL > last_lat)
        {
            last_lat = min(LAT_MULT_NUM_ELEMENTS - 1, abs(t_lat / GNSS_MUL));
            step_y = (zoom_p * (int64_t)GNSS_MUL / lat_mult[last_lat]) / MAP_DIV_CONST;
            last_map_h = map_h;
            map_h = (MAP_H - 2) * step_y;
        }
    }

    *c_lat = t_lat + last_map_h / 2;
}

//get degrees for one pixel
void geo_get_steps(int32_t lat, uint16_t zoom, int32_t * step_x, int32_t * step_y)
{
    uint16_t zoom_p = pow(2, min(zoom, 9));

	*step_x = (zoom_p * (int64_t)GNSS_MUL) / MAP_DIV_CONST;
//    *step_y = (zoom_p * (int64_t)GNSS_MUL) / MAP_DIV_CONST;
//    return;
	uint8_t lat_i = min(LAT_MULT_NUM_ELEMENTS - 1, abs(lat / GNSS_MUL));
	*step_y = (zoom_p * (int64_t)GNSS_MUL / lat_mult[lat_i]) / MAP_DIV_CONST;
}


void geo_get_topo_steps(int32_t lat, int32_t step_x, int32_t step_y, int16_t * step_x_m, int16_t * step_y_m)
{
	uint8_t lat_i = min(LAT_MULT_NUM_ELEMENTS - 1, abs(lat / GNSS_MUL));
    *step_x_m = max(1, (int64_t)step_x * 111000l / GNSS_MUL);
    *step_y_m = max(1, (int64_t)step_y * 111000l / GNSS_MUL * lat_mult[lat_i]);
}

void geo_destination_f(float lat1, float lon1, float angle, float distance_km, float * lat2, float * lon2)
{
	angle = to_radians(angle);
	float dx = sin(angle) * distance_km;
	float dy = cos(angle) * distance_km;

	float kx, ky;
	get_kx_ky(lat1, &kx, &ky);

	*lon2 = lon1 + dx / kx;
	*lat2 = lat1 + dy / ky;
}

void geo_destination(int32_t lat1, int32_t lon1, float angle, float distance_km, int32_t * lat2, int32_t * lon2)
{
    angle = to_radians(angle);
    float dx = sin(angle) * distance_km;
    float dy = cos(angle) * distance_km;

    float kx, ky;
    get_kx_ky(lat1 / GNSS_MUL, &kx, &ky);

    *lon2 = lon1 + (dx / kx) * GNSS_MUL;
    *lat2 = lat1 + (dy / ky) * GNSS_MUL;
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
 * \return the distance in cm.
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

		dist = 2 * FAI_EARTH_RADIUS * asin(sqrt(q)) * 100000.0;
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

        dist = sqrt(pow(d_lon, 2) + pow(d_lat, 2)) * 100000.0;
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

bool lines_common_point(int32_t x1, int32_t y1, int32_t x2, int32_t y2,
		int32_t x3, int32_t y3, int32_t x4, int32_t y4,
		int32_t * px, int32_t * py)
{
	int64_t d = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);

	if (d == 0)
		return false;

	*px = ((x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4)) / d;
	*py = ((x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4))/ d;

    return true;
}

bool point_in_box(int32_t px, int32_t py, int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
	if (x1 > x2)
	{
		int32_t t = x2;
		x2 = x1;
		x1 = t;
	}

	if (y1 > y2)
	{
		int32_t t = y2;
		y2 = y1;
		y1 = t;
	}

    return px >= x1 && px <= x2 && py >= y1 && py <= y2;
}

bool lines_intersection(int32_t x1, int32_t y1, int32_t x2, int32_t y2,
		int32_t x3, int32_t y3, int32_t x4, int32_t y4,
		int32_t * px, int32_t * py)
{
	if (!lines_common_point(x1, y1, x2, y2, x3, y3, x4, y4, px, py))
		//lines are parallel
		return false;

	return point_in_box(*px, *py, x1, y1, x2, y2) && point_in_box(*px, *py, x3, y3, x4, y4);
}


