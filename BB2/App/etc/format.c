#include "format.h"

#include "../config/config.h"
#include "../fc/fc.h"


void format_gnss_datum(char * slat, char * slon, int32_t lat, int32_t lon)
{
	uint32_t alat = abs(lat);
	uint32_t alon = abs(lon);

	switch (config_get_select(&config.settings.units.geo_datum))
	{
		case(GNSS_DDdddddd):
			sprintf(slat, "+%0.6f", lat / (float)GNSS_MUL);
			sprintf(slon, "+%0.6f", lon / (float)GNSS_MUL);
		break;

		case(GNSS_DDMMmmm):
			sprintf(slat, "%lu°%6.3f' %c", alat / GNSS_MUL, ((alat % GNSS_MUL) * 60) / (float)GNSS_MUL, lat > 0 ? 'N' : 'S');
			sprintf(slon, "%lu°%6.3f' %c", alon / GNSS_MUL, ((alon % GNSS_MUL) * 60) / (float)GNSS_MUL, lon > 0 ? 'E' : 'W');
		break;

		case(GNSS_DDMMSS):
			sprintf(slat, "%lu°%02lu'%02.4f\" %c", alat / GNSS_MUL, ((alat % GNSS_MUL) * 60) / GNSS_MUL, ((alat % 100000) * 60) / 100000.0, lat > 0 ? 'N' : 'S');
			sprintf(slon, "%lu°%02lu'%02.4f\" %c", alon / GNSS_MUL, ((alon % GNSS_MUL) * 60) / GNSS_MUL, ((alon % 100000) * 60) / 100000.0, lon > 0 ? 'E' : 'W');
		break;
	}
}

void format_distance(char * buf, float in)
{
	switch (config_get_select(&config.settings.units.distance))
	{
		case(DISTANCE_METERS):
			if (in < 1000) //1km
				sprintf(buf, "%0.0fm", in);
			else if (in < 10000) //10km
				sprintf(buf, "%0.2fkm", in / 1000.0);
			else if (in < 100000) //100km
				sprintf(buf, "%0.1fkm", in / 1000.0);
			else
				sprintf(buf, "%0.0fkm", in / 1000.0);
		break;

		case(DISTANCE_MILES):
		{
			float mi = (in / 1000.0) * FC_KM_TO_MILE;
			if (in < 1.0) //1mi
				sprintf(buf, "%0.2fmi", mi);
			if (in < 10.0) //10mi
				sprintf(buf, "%0.1fmi", mi);
			else
				sprintf(buf, "%0.0fmi", mi);

		}
		break;

	}
}
