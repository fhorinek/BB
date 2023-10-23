#include "format.h"

#include "fc/fc.h"
#include "etc/epoch.h"
#include "drivers/rev.h"
#include "etc/geo_calc.h"

char *month_names[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

void format_date_epoch(char * buff, uint64_t epoch)
{
	uint8_t sec, min, hour, day, wday, month;
	uint16_t year;

	datetime_from_epoch(epoch, &sec, &min, &hour, &day, &wday, &month, &year);
	format_date(buff, day, month, year);
}

void format_date(char * buff, uint8_t day, uint8_t month, uint16_t year)
{
    switch (config_get_select(&config.units.date))
    {
        case(DATE_DDMMYYYY):
            sprintf(buff, "%02u.%02u.%04u", day, month, year);
        break;
        case(DATE_MMDDYYYY):
            sprintf(buff, "%02u/%02u/%04u", month, day, year);
        break;
        case(DATE_YYYYMMDD):
            sprintf(buff, "%04u-%02u-%02u", year, day, month);
        break;

    }
}

void format_date_DM(char * buff, uint8_t day, uint8_t month)
{
    switch (config_get_select(&config.units.date))
    {
        case(DATE_DDMMYYYY):
            sprintf(buff, "%02u.%02u.", day, month);
        break;
        case(DATE_MMDDYYYY):
            sprintf(buff, "%02u/%02u", month, day);
        break;
        case(DATE_YYYYMMDD):
            sprintf(buff, "%02u-%02u", day, month);
        break;
    }
}


/**
 * Give a time representation of given hour and min
 * depending on time format choosen by user:
 * "23:59" or "11:59 pm"
 *
 * @param buff the buffer receiving the result (at least 9 bytes including \0)
 * @param hour the hour to print
 * @param min the minute to print
 */
void format_time(char * buff, uint8_t hour, uint8_t min)
{
    if (config_get_bool(&config.units.time24))
    {
        sprintf(buff, "%02u:%02u", hour, min);
    }
    else
    {
        char c = 'a';
        if (hour >= 12)
        {
            c = 'p';
            if (hour > 13)
                hour -= 12;
        }

        sprintf(buff, "%u:%02u %cm", hour, min, c);
    }
}

void format_gnss_datum(char * slat, char * slon, int32_t lat, int32_t lon)
{
	uint32_t alat = abs(lat);
	uint32_t alon = abs(lon);

	switch (config_get_select(&config.units.geo_datum))
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

/**
 * Format a vario value into a string for display to user, e.g. "2.0".
 * There is no unit after the number.
 *
 * @param val the buffer receiving the output
 * @param in the vario value in "meter/s"
 */
void format_vario(char * val, float in)
{
    int16_t value;

    switch (config_get_select(&config.units.vario))
    {
        case(VARIO_MPS):
        default:
            value = in * 10;
        break;
        case(VARIO_KN):
            value = in * FC_MPS_TO_KNOTS * 10;
        break;
        case(VARIO_FPM):
            value = in * FC_MPS_TO_100FPM * 10;
        break;
    }

    sprintf(val, "%0.1f", value / 10.0);
}

/**
 * Format a vario value's unit into a string for display to user, e.g. "m/s".
 *
 * @param val the buffer receiving the output
 */
void format_vario_units(char * units)
{
    switch (config_get_select(&config.units.vario))
    {
        case(VARIO_MPS):
            strcpy(units, "m/s");
        break;
        case(VARIO_KN):
            strcpy(units, "knots");
        break;
        case(VARIO_FPM):
            strcpy(units, "100ft/m");
        break;
    }
}

/**
 * Format a vario value into a string for display to user, e.g. "2.0 m/s".
 *
 * @param val the buffer receiving the output
 * @param in the vario value in "meter/s"
 */
void format_vario_with_units(char * buff, float in)
{
	char val[16];
	char units[16];
	format_vario(val, in);
	format_vario_units(units);
	sprintf(buff, "%s %s", val, units);
}

void format_altitude_2(char * buff, float in, uint8_t units)
{
    int16_t val;

    switch (units)
    {
        case(ALTITUDE_FT):
            val = FC_METER_TO_FEET * in;
        break;

        case(ALTITUDE_M):
        default:
            val = in;
        break;
    }

    sprintf(buff, "%d", val);
}


void format_altitude(char * buff, float in)
{
    int32_t val;

    switch (config_get_select(&config.units.altitude))
    {
        case(ALTITUDE_FT):
            val = FC_METER_TO_FEET * in;
        break;

        case(ALTITUDE_M):
        default:
            val = in;
        break;
    }

    sprintf(buff, "%d", val);
}

void format_altitude_gain_2(char * buff, float in, uint8_t units)
{
    int32_t val;

    switch (units)
    {
        case(ALTITUDE_FT):
            val = FC_METER_TO_FEET * in;
        break;

        case(ALTITUDE_M):
        default:
            val = in;
        break;
    }

    sprintf(buff, "%+d", val);
}


void format_altitude_gain(char * buff, float in)
{
    int32_t val;

    switch (config_get_select(&config.units.altitude))
    {
        case(ALTITUDE_FT):
            val = FC_METER_TO_FEET * in;
        break;

        case(ALTITUDE_M):
        default:
            val = in;
        break;

    }

    sprintf(buff, "%+d", val);
}

void format_altitude_units(char * buff)
{
    switch (config_get_select(&config.units.altitude))
    {
        case(ALTITUDE_M):
            strcpy(buff, "m");
        break;
        case(ALTITUDE_FT):
            strcpy(buff, "ft");
        break;
    }
}

void format_altitude_units_2(char * buff, uint8_t units)
{
    switch (units)
    {
        case(ALTITUDE_M):
            strcpy(buff, "m");
        break;
        case(ALTITUDE_FT):
            strcpy(buff, "ft");
        break;
    }
}

void format_altitude_with_units(char * buff, float in)
{
	char val[16];
	char units[16];
	format_altitude(val, in);
	format_altitude_units(units);
	sprintf(buff, "%s %s", val, units);
}

void format_altitude_with_units_2(char * buff, float in, uint8_t units)
{
    char val[16];
    char uni[16];
    format_altitude_2(val, in, units);
    format_altitude_units_2(uni, units);
    sprintf(buff, "%s %s", val, uni);
}

void format_FL_with_altitude_with_units(char * buff, float in)
{
    char val[16];
    char uni[4];
    format_altitude(val, (in * 100) / FC_METER_TO_FEET);
    format_altitude_units(uni);
    uint16_t in_i = in;
    sprintf(buff, "FL%03u %s%s", in_i, val, uni);
}
/**
 * Format a distance according to user configuration, e.g. "10.34km"
 *
 * @param buf the buffer receiving the string representation
 * @param in the distance in meter.
 */
void format_distance_with_units(char * buf, float in)
{
    switch (config_get_select(&config.units.distance))
    {
        case(DISTANCE_METERS):
            if (in < 1000) //1km
                sprintf(buf, "%0.0f m", in);
            else if (in < 10000) //10km
                sprintf(buf, "%0.2f km", in / 1000.0);
            else if (in < 100000) //100km
                sprintf(buf, "%0.1f km", in / 1000.0);
            else
                sprintf(buf, "%0.0f km", in / 1000.0);
        break;

        case(DISTANCE_MILES):
        {
            float mi = (in / 1000.0) * FC_KM_TO_MILE;
            if (in < 1.0) //1mi
                sprintf(buf, "%0.0f ft", mi * 5280);
            if (in < 10.0) //10mi
                sprintf(buf, "%0.1f mi", mi);
            else
                sprintf(buf, "%0.0f mi", mi);
        }
        break;

    }
}

void format_distance_with_units2(char * buf, float in)
{
	switch (config_get_select(&config.units.distance))
	{
		case(DISTANCE_METERS):
			if (in < 1000) //1km
				sprintf(buf, "%0.0f m", in);
			else
				sprintf(buf, "%0.0f km", in / 1000.0);
		break;

		case(DISTANCE_MILES):
		{
			float mi = (in / 1000.0) * FC_KM_TO_MILE;
			if (in < 1.0) //1mi
				sprintf(buf, "%0.0f ft", mi * 5280);
			else
				sprintf(buf, "%0.0f mi", mi);
		}
		break;

	}
}

void format_distance(char * buf, float in)
{
	switch (config_get_select(&config.units.distance))
	{
		case(DISTANCE_METERS):
			if (in < 1000) //1km
				sprintf(buf, "%0.0f", in);
			else if (in < 10000) //10km
				sprintf(buf, "%0.2f", in / 1000.0);
			else if (in < 100000) //100km
				sprintf(buf, "%0.1f", in / 1000.0);
			else
				sprintf(buf, "%0.0f", in / 1000.0);
		break;

		case(DISTANCE_MILES):
		{
			float mi = (in / 1000.0) * FC_KM_TO_MILE;
			if (mi < 1.0) //1mi
				sprintf(buf, "%0.0f", mi * 5280);
			if (mi < 10.0) //10mi
				sprintf(buf, "%0.1f", mi);
			else
				sprintf(buf, "%0.0f", mi);
		}
		break;

	}
}

/**
 * Select the appropriate unit for the given distance.
 *
 * @param buf a character buffer receiving the unit.
 * @param in the distance in meter.
 */
void format_distance_units(char * buf, float in)
{
	switch (config_get_select(&config.units.distance))
	{
		case(DISTANCE_METERS):
			if (in < 1000) //1km
				strcpy(buf, "m");
			else
				strcpy(buf, "km");
		break;

		case(DISTANCE_MILES):
		{
			float mi = (in / 1000.0) * FC_KM_TO_MILE;
			if (mi < 1.0) //1mi
				strcpy(buf, "ft");
			else
				strcpy(buf, "mi");
		}
		break;

	}
}


void format_mac(char * buf, uint8_t * mac)
{
    for(uint8_t i = 0; i < 6; i++)
    {
        sprintf(buf + i * 3, "%02X:", mac[i]);
    }

    buf[3 * 6 - 1] = 0;
}

void parse_mac(const char * mac_str, uint8_t * mac)
{
    for (uint8_t i = 0; i < 6; i++)
    {
        char byte_str[3];
        strncpy(byte_str, mac_str + i * 3, 2);
        byte_str[2] = '\0';
        mac[i] = (uint8_t)strtol(byte_str, NULL, 16);
    }
}


void format_ip(char * buf, uint8_t * ip)
{
    sprintf(buf, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
}

void format_speed_2(char * val, float in, char * format)
{
    float value;

    switch (config_get_select(&config.units.speed))
    {
        case(SPEED_KMH):
        default:
            value = in * FC_MPS_TO_KPH;
        break;
        case(SPEED_MPH):
            value = in * FC_MPS_TO_MPH;
        break;
        case(SPEED_MPS):
            value = in;
        break;
        case(SPEED_KNOTS):
            value = in * FC_MPS_TO_KNOTS;
        break;
    }

    sprintf(val, format, value);
}

void format_speed(char * val, float in)
{
    float value;

    switch (config_get_select(&config.units.speed))
    {
        case(SPEED_KMH):
        default:
            value = in * FC_MPS_TO_KPH;
        break;
        case(SPEED_MPH):
            value = in * FC_MPS_TO_MPH;
        break;
        case(SPEED_MPS):
            value = in;
        break;
        case(SPEED_KNOTS):
            value = in * FC_MPS_TO_KNOTS;
        break;
    }

    sprintf(val, "%0.1f", value);
}

void format_speed_units(char * units)
{
    switch (config_get_select(&config.units.speed))
    {
        case(SPEED_KMH):
            strcpy(units, "km/h");
        break;
        case(SPEED_MPH):
            strcpy(units, "mi/h");
        break;
        case(SPEED_MPS):
            strcpy(units, "m/s");
        break;
        case(SPEED_KNOTS):
            strcpy(units, "kt");
        break;
    }
}

void format_speed_with_units(char * buff, float in)
{
	char val[16];
	char units[16];
	format_speed(val, in);
	format_speed_units(units);
	sprintf(buff, "%s %s", val, units);
}


void format_percent(char * buff, float in)
{
	uint8_t val = min(in, 100);
    sprintf(buff, "%u%%", val);
}

void format_duration(char * buff, float in)
{
	int16_t sec = in;

	if (sec > 60)
	{
		if (sec > 3600)
		{
			sprintf(buff, "%uh %um", sec / 3600, (sec % 3600) / 60);
		}
		else
		{
			sprintf(buff, "%um %us", sec / 60, sec % 60);
		}
	}
	else
	{
		sprintf(buff, "%us", sec);
	}
}

void format_duration_sec_f(char * buff, float in)
{
    sprintf(buff, "%0.1fs", in);
}

void format_uuid(char * buff)
{
	uint8_t uuid[12];
	rev_get_uuid(uuid);

	for (uint8_t i = 0; i < 12; i++)
		sprintf(buff + (i * 2), "%02X", uuid[i]);
}

void format_int(char * buff, float in)
{
	int32_t i = in;
	sprintf(buff, "%ld", i);
}


void format_hdg_to_points(char * buff, float in)
{
	if (22.5 <= in && in < 67.5)
		strcpy(buff, "NE");
	else if (67.5 <= in && in < 112.5)
		strcpy(buff, "E");
	else if (112.5 <= in && in < 157.5)
		strcpy(buff, "SE");
	else if (157.5 <= in && in < 202.5)
		strcpy(buff, "S");
	else if (202.5 <= in && in < 247.5)
		strcpy(buff, "SW");
	else if (247.5 <= in && in < 292.5)
		strcpy(buff, "W");
	else if (292.5 <= in && in < 337.5)
		strcpy(buff, "NW");
	else
		strcpy(buff, "N");
}

void format_zoom(char *buff, float in)
{
    int16_t zoom = in;
    uint16_t zoom_p = pow(2, zoom);
    float guide_m = (zoom_p * 111000 * 120 / MAP_DIV_CONST);
    format_distance_with_units2(buff, guide_m);
}
