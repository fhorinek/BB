/*
 * airspace.cpp
 *
 *  Created on: Mar 14, 2022
 *      Author: horinek
 */
#define DEBUG_LEVEL	DBG_DEBUG

#include "airspace.h"
#include "fc/fc.h"
#include "etc/geo_calc.h"
#include "gui/dialog.h"
#include "gui/statusbar.h"
#include "gui/tasks/filemanager.h"

void airspace_create_lock()
{
    fc.airspaces.lock = osMutexNew(NULL);
    osMutexAcquire(fc.airspaces.lock, WAIT_INF);
    vQueueAddToRegistry(gui.lock, "airspace.lock");
    osMutexRelease(fc.airspaces.lock);
}

bool airspace_point(char * line, int32_t * lon, int32_t * lat)
{
    unsigned int lat_deg, lat_min, lat_sec, lat_dsec;
    unsigned int lon_deg, lon_min, lon_sec, lon_dsec;
    char lat_c, lon_c;

    uint8_t filled = sscanf(line, "%02u:%02u:%02u.%02u %c %03u:%02u:%02u.%02u %c",
                        &lat_deg, &lat_min, &lat_sec, &lat_dsec, &lat_c,
                        &lon_deg, &lon_min, &lon_sec, &lon_dsec, &lon_c);

    if (filled != 10)
    {
        lat_dsec = 0;
        lon_dsec = 0;
        filled = sscanf(line, "%02u:%02u:%02u %c %03u:%02u:%02u %c",
                        &lat_deg, &lat_min, &lat_sec, &lat_c,
                        &lon_deg, &lon_min, &lon_sec, &lon_c);

        if (filled != 8)
        {
            WARN("unable to read geo point %s", line);
            return false;
        }
    }

    *lat = lat_deg * GNSS_MUL + lat_min * (GNSS_MUL / 60) + lat_sec * (GNSS_MUL / 3600) + lat_dsec * (GNSS_MUL / 360000);
    if (lat_c == 'S') *lat *= -1;
    *lon = lon_deg * GNSS_MUL + lon_min * (GNSS_MUL / 60) + lon_sec * (GNSS_MUL / 3600) + lon_dsec * (GNSS_MUL / 360000);
    if (lon_c == 'W') *lon *= -1;

    if (abs(*lat) > 90 * GNSS_MUL || abs(*lon) > 180 * GNSS_MUL)
    {
        WARN("unable to read geo point %s", line);
        return false;
    }

    return true;
}

uint16_t airspace_alt(char * line, bool * gnd)
{
    bool fl = false;

    if (strstr(line, "GND") != NULL)
        *gnd = true;

    if (strncmp("FL", line, 2) == 0)
    {
        line += 2;
        fl = true;
    }

    uint16_t value = atoi_c(line);

    if (fl)
    {
        value = (value * 100) / FC_METER_TO_FEET;
        *gnd = false;
    }

    if (strstr(line, "ft") != NULL)
        value /= FC_METER_TO_FEET;

    if (strstr(line, "AMSL") != NULL)
        *gnd = false;

    return value;
}

//transform linked list to normal allocated array
//calculate bounding box
//return used memory
uint32_t airspace_finalise(airspace_record_t * as)
{
	if (as->number_of_points)
	{
		as->bbox.latitude1 = -90 * GNSS_MUL;
		as->bbox.latitude2 = +90 * GNSS_MUL;
		as->bbox.longitude1 = +180 * GNSS_MUL;
		as->bbox.longitude2 = -180 * GNSS_MUL;

		gnss_pos_list_t * actual = (gnss_pos_list_t *)as->points;

		as->points = (gnss_pos_t *) ps_malloc(as->number_of_points * sizeof(gnss_pos_t));

		for (uint32_t i = 0; i < as->number_of_points; i++)
		{
			ASSERT(actual != NULL);

			as->points[i].latitude = actual->latitude;
			as->points[i].longitude = actual->longitude;

//			RAW("rectangle bbox=%f,%f,%f,%f target=airspace", actual->longitude / (float)GNSS_MUL, actual->latitude / (float)GNSS_MUL, -0.001 + actual->longitude / (float)GNSS_MUL, 0.001 + actual->latitude / (float)GNSS_MUL);

			as->bbox.latitude1 = max(as->bbox.latitude1, actual->latitude);
			as->bbox.latitude2 = min(as->bbox.latitude2, actual->latitude);
			as->bbox.longitude1 = min(as->bbox.longitude1, actual->longitude);
			as->bbox.longitude2 = max(as->bbox.longitude2, actual->longitude);

			gnss_pos_list_t * last = actual;
			actual = actual->next;
			tfree(last);
		}
//		RAW("rectangle bbox=%f,%f,%f,%f name=bbox", as->bbox.longitude1 / (float)GNSS_MUL, as->bbox.latitude1 / (float)GNSS_MUL, -0.001 + as->bbox.longitude2 / (float)GNSS_MUL, 0.001 + as->bbox.latitude2 / (float)GNSS_MUL);
	}
//	DBG(">%s", as->name);
//	DBG(" %u points", as->number_of_points);
//	DBG(" %u pen width", as->pen_width & PEN_WIDTH_MASK);
//	DBG(" %u %u %u pen color", as->pen.ch.red, as->pen.ch.green, as->pen.ch.blue);
//	if (as->pen_width & BRUSH_TRANSPARENT_FLAG)
//		DBG("transparent brush");
//	else
//		DBG(" %u %u %u brush", as->brush.ch.red, as->brush.ch.green, as->brush.ch.blue);
//	DBG("\n");

	return sizeof(airspace_record_t)
			+ as->number_of_points * sizeof(gnss_pos_t)
			+ ((as->name != NULL) ? strlen(as->name) + 1 : 0);
}

void airspace_add_point(airspace_record_t * as, gnss_pos_list_t ** pos_list, int32_t lon, int32_t lat)
{
	gnss_pos_list_t * new = (gnss_pos_list_t *) tmalloc(sizeof(gnss_pos_list_t));

	if (new == NULL)
	{
	    bsod_msg("Unable to allocate more memory, File is too big");
	}

	new->latitude = lat;
	new->longitude = lon;
	new->next = NULL;

	as->number_of_points++;

	if (*pos_list == NULL)
	{
		*pos_list = new;
		//use points as first item
		as->points = (gnss_pos_t *)new;
	}
	else
	{
		(*pos_list)->next = new;
		*pos_list = new;
	}
}

//free airspace list
//as - head of the list
void airspace_free(airspace_record_t * as)
{
	airspace_record_t * actual = as;
	while (actual != NULL)
	{
		airspace_record_t * last = actual;
		actual = actual->next;

		if (last->name != NULL)
			ps_free(last->name);

		if (last->number_of_points > 0)
			ps_free(last->points);

		ps_free(last);
	}
}

airspace_record_t * airspace_load_cache(char * orig, uint16_t * loaded, uint16_t * hidden, uint32_t * mem_used);
void airspace_save_cache(char * orig, airspace_record_t * first, uint16_t loaded, uint16_t hidden);

airspace_record_t * airspace_load(char * path, uint16_t * loaded, uint16_t * hidden, uint32_t * mem_used, bool use_dialog)
{
    INFO("airspace_load %s", path);

    airspace_record_t * first = NULL;

    *loaded = 0;
    *hidden = 0;
    *mem_used = 0;

    first = airspace_load_cache(path, loaded, hidden, mem_used);

    if (first != NULL)
    {
        return first;
    }

    int f = red_open(path, RED_O_RDONLY);

    if (f > 0)
    {
        lv_obj_t * msg_ptr = NULL;

        if (!use_dialog)
        {
            msg_ptr = statusbar_msg_add(STATUSBAR_MSG_PROGRESS, "Loading airspaces");
        }


    	uint32_t size = file_size(f);
    	uint32_t pos = 0;
    	uint8_t skip = 0;
    	char mem_line[128];
        char * line;

        airspace_record_t * actual = NULL;
        airspace_record_t * prev = NULL;
        gnss_pos_list_t * points_last = NULL;
        gnss_pos_t center = {0};
        bool clockwise = true;

        while (true)
        {
            if ((line = red_gets(mem_line, sizeof(mem_line), f)) == NULL)
                break;

            if (line == GETS_CORRUPTED)
            {
                statusbar_msg_add(STATUSBAR_MSG_ERROR, "Airspace file corrupted");
                break;
            }

            pos += strlen(line);

            if (skip == 0)
            {
                if (use_dialog)
                {
                    dialog_progress_set_progress((pos * 100) / size);
                }
                else
                {
                    statusbar_msg_update_progress(msg_ptr, (pos * 100) / size);
                }
            }
            skip = (skip + 1) % 20;

            DBG("AIR: %s", line);

            if (line[0] == '*' || line[0] == '\r' || strlen(line) == 0)
                continue;

            //airspace class
            if (strncmp("AC ", line, 3) == 0)
            {
				if (actual != NULL)
				{
					if (actual->airspace_class == ac_hidden || actual->number_of_points == 0)
					{
						//reuse the old one
						if (actual->name != NULL)
						{
							ps_free(actual->name);
						}

						if (actual->number_of_points > 0)
						{
						    //clear points if hidden
						    gnss_pos_list_t * actual_point = (gnss_pos_list_t *)actual->points;

							for (uint32_t i = 0; i < actual->number_of_points; i++)
					        {
					            gnss_pos_list_t * last = actual_point;
					            actual_point = actual_point->next;
					            tfree(last);
					        }
						}

						(*hidden)++;
					}
					else
					{
						//finalize last airspace here, allocate new
						(*mem_used) += airspace_finalise(actual);

						actual->next = ps_malloc(sizeof(airspace_record_t));
						prev = actual;
						actual = actual->next;
					}
				}

                if (first == NULL)
                {
                	actual = ps_malloc(sizeof(airspace_record_t));
                    first = actual;
                }

                //reset airspace
//                tmalloc_tag_inc();

                actual->name = NULL;
                actual->points = NULL;
                actual->number_of_points = 0;
                actual->next = NULL;
                actual->pen_width = 0;
                actual->pen.full = 0;
                actual->brush.full = 0;
                clockwise = true;

                points_last = NULL;

                line += 3;
                if (strncmp(line, "GP", 2) == 0)
                    actual->airspace_class = config_get_bool(&profile.airspace.display.prohibited) ? ac_glider_prohibited : ac_hidden;
                else if (strncmp(line, "CTR", 3) == 0)
                    actual->airspace_class = config_get_bool(&profile.airspace.display.ctr) ? ac_ctr : ac_hidden;
                else if (strncmp(line, "TMZ", 3) == 0)
                    actual->airspace_class = config_get_bool(&profile.airspace.display.tmz) ? ac_tmz : ac_hidden;
                else if (strncmp(line, "RMZ", 3) == 0)
                    actual->airspace_class = config_get_bool(&profile.airspace.display.rmz) ? ac_rmz : ac_hidden;
                else if (*line == 'R')
                     actual->airspace_class = config_get_bool(&profile.airspace.display.restricted) ? ac_restricted : ac_hidden;
                else if (*line == 'Q')
                    actual->airspace_class = config_get_bool(&profile.airspace.display.danger) ? ac_danger : ac_hidden;
                else if (*line == 'P')
                    actual->airspace_class = config_get_bool(&profile.airspace.display.prohibited) ? ac_prohibited : ac_hidden;
                else if (*line == 'A')
                    actual->airspace_class = config_get_bool(&profile.airspace.display.class_A) ? ac_class_A : ac_hidden;
                else if (*line == 'B')
                    actual->airspace_class = config_get_bool(&profile.airspace.display.class_B) ? ac_class_B : ac_hidden;
                else if (*line == 'C')
                    actual->airspace_class = config_get_bool(&profile.airspace.display.class_C) ? ac_class_C : ac_hidden;
                else if (*line == 'D')
                    actual->airspace_class = config_get_bool(&profile.airspace.display.class_D) ? ac_class_D : ac_hidden;
                else if (*line == 'E')
                    actual->airspace_class = config_get_bool(&profile.airspace.display.class_E) ? ac_class_E : ac_hidden;
                else if (*line == 'F')
                    actual->airspace_class = config_get_bool(&profile.airspace.display.class_F) ? ac_class_F : ac_hidden;
                else if (*line == 'G')
                    actual->airspace_class = config_get_bool(&profile.airspace.display.class_G) ? ac_class_G : ac_hidden;
                else if (*line == 'W')
                    actual->airspace_class = config_get_bool(&profile.airspace.display.wave_window) ? ac_wave_window : ac_hidden;
               else
                    actual->airspace_class = config_get_bool(&profile.airspace.display.undefined) ? ac_undefined : ac_hidden;

                (*loaded)++;
                continue;
            }
            else if (actual == NULL)
            {
            	ERR("No Class defined, skipping");
            	continue;
            }
            else if (actual->airspace_class == ac_hidden)
            {
            	continue;
            }
            //airspace name
            else if (strncmp("AN ", line, 3) == 0)
            {
                line += 3;

                if (actual->name != NULL)
                {
                    ASSERT(0);
                    ps_free(actual->name);
                }

                uint16_t len = min(AIRSPACE_NAME_LEN, strlen_noend(line) + 1);
                actual->name = ps_malloc(len);
                strncpy(actual->name, line, len - 1);
                actual->name[len - 1] = 0;
                continue;
            }
            //Airspace ceiling
            else if (strncmp("AH ", line, 3) == 0)
            {
                line += 3;

                actual->ceil = airspace_alt(line, &actual->ceil_gnd);
            }
            //Airspace floor
            else if (strncmp("AL ", line, 3) == 0)
            {
                line += 3;

                actual->floor = airspace_alt(line, &actual->floor_gnd);
                if (actual->floor > (config_get_int(&profile.airspace.display.below) * 100) / FC_METER_TO_FEET)
                {
                	(*hidden)++;
                	actual->airspace_class = ac_hidden;
                }
            }
            //Pen
            else if (strncmp("SP ", line, 3) == 0)
            {
                line += 3;

                //skip style
                line = find_comma(line);
                //pen width
                actual->pen_width = atoi_c(line) & PEN_WIDTH_MASK;
                line = find_comma(line);
                //pen red
                uint8_t red = atoi_c(line);
                line = find_comma(line);
                //pen green
                uint8_t green = atoi_c(line);
                line = find_comma(line);
                //pen blue
                uint8_t blue = atoi_c(line);

                actual->pen = LV_COLOR_MAKE(red, green, blue);
            }
            //Brush
            else if (strncmp("SB ", line, 3) == 0)
            {
                line += 3;

                if (*line == '-') //-1,-1,-1
                {
                	actual->pen_width |= BRUSH_TRANSPARENT_FLAG;
                }
                else
                {
					//brush red
					uint8_t red = atoi_c(line);
					line = find_comma(line);
					//brush green
					uint8_t green = atoi_c(line);
					line = find_comma(line);
					//brush blue
					uint8_t blue = atoi_c(line);

					actual->brush = LV_COLOR_MAKE(red, green, blue);
                }
            }
            //polygon point
            else if (strncmp("DP ", line, 3) == 0)
            {
                line += 3;

                int32_t lon, lat;

                if (airspace_point(line, &lon, &lat))
                {
                	airspace_add_point(actual, &points_last, lon, lat);
                }
            }
            //center point
            else if (strncmp("V X=", line, 4) == 0)
            {
                line += 4;

                int32_t lon, lat;

                if (airspace_point(line, &lon, &lat))
                {
                	center.latitude = lat;
                	center.longitude = lon;
                }
            }
            //circle direction
			else if (strncmp("V D=", line, 4) == 0)
			{
				line += 4;
				if (*line == '-')
					clockwise = false;
				if (*line == '+')
					clockwise = true;
			}
            //draw arc
            else if (strncmp("DA ", line, 3) == 0)
            {
                line += 3;

                uint16_t radius = atoi_c(line);
                float distance_km = radius * FC_NM_TO_KM;
                line = find_comma(line);
                int16_t start = atoi_c(line);
                int16_t end = atoi_c(line);

                int16_t dir = clockwise ? +5 : -5;

                bool done = false;
                //DBG("start %d, end %d, dir %d", start, end, dir);
                for (int16_t angle = start; !done;)
                {
                	int32_t lon, lat;

                	//DBG("angle %d", angle);

                	if (clockwise)
                	{
                		if (angle >= end)
                		{
                			angle = end;
                			done = true;
                		}
                	}
                	else
                	{
                		if (angle <= end)
                		{
                			angle = end;
                			done = true;
                		}
                	}

                    if (angle > 360)
                    {
                        angle %= 360;
                    }

                	geo_destination(center.latitude, center.longitude, angle, distance_km, &lat, &lon);
                	airspace_add_point(actual, &points_last, lon, lat);

                    angle += dir;
                }
            }
            //draw arc2
            else if (strncmp("DB ", line, 3) == 0)
            {
                line += 3;

                int32_t lon1, lat1, lon2, lat2;

                if (!airspace_point(line, &lon1, &lat1))
                	continue;

                line = find_comma(line);

                if (!airspace_point(line, &lon2, &lat2))
                	continue;

                int16_t start;
                int16_t end;

                float distance_km = geo_distance(center.latitude, center.longitude, lat1, lon1, config_get_select(&config.units.earth_model) == EARTH_FAI, &start) / 100000.0;
                geo_distance(center.latitude, center.longitude, lat2, lon2, config_get_select(&config.units.earth_model), &end);

                int16_t dir = clockwise ? +5 : -5;

                if (clockwise)
                {
                    while (start > end)
                        end += 360;
                }
                else
                {
                    while (start < end)
                        start += 360;
                }

                bool done = false;
//                DBG("start %d, end %d, dir %d", start, end, dir);
                for (int16_t angle = start; !done; )
                {
                	int32_t lon, lat;

                	if (clockwise)
                	{
                		if (angle >= end)
                		{
                			angle = end;
                			done = true;
                		}
                	}
                	else
                	{
                		if (angle <= end)
                		{
                			angle = end;
                			done = true;
                		}
                	}

                	geo_destination(center.latitude, center.longitude, angle, distance_km, &lat, &lon);
                	airspace_add_point(actual, &points_last, lon, lat);

                    angle += dir;
                }
            }
            //draw circle
            else if (strncmp("DC ", line, 3) == 0)
            {
                line += 3;

                float radius = atoi_f(line);
                float distance_km = radius * FC_NM_TO_KM;

                for (uint16_t angle = 0; angle < 360; angle += 5)
                {
                	int32_t lon, lat;

                	geo_destination(center.latitude, center.longitude, angle, distance_km, &lat, &lon);
                	airspace_add_point(actual, &points_last, lon, lat);
                }
            }

        }

		//finalize last airspace here
        if (actual != NULL)
        {
            if (actual->airspace_class == ac_hidden || actual->number_of_points == 0)
            {
                prev->next = NULL;

                if (actual->name != NULL)
                {
                    ps_free(actual->name);
                }

                //clear points
                if (actual->number_of_points)
                {
                    gnss_pos_list_t * actual_point = (gnss_pos_list_t *)actual->points;

                    for (uint32_t i = 0; i < actual->number_of_points; i++)
                    {
                        gnss_pos_list_t * last = actual_point;
                        actual_point = actual_point->next;
                        tfree(last);
                    }
                }
                ps_free(actual);

                (*hidden)++;
            }
            else
            {
                (*mem_used) += airspace_finalise(actual);
            }
        }
		red_close(f);

		airspace_save_cache(path, first, *loaded, *hidden);

		if (msg_ptr != NULL)
		{
		    statusbar_msg_close(msg_ptr);
		}
    }
    else
    {
        ERR("Unable to open %s, err = %u", path, f);
    }

    INFO("Loaded %u airspaces (%u hidden)", *loaded, *hidden);
    INFO(" used %u bytes", *mem_used);

    return first;
}

#define CACHE_VERSION       15

uint8_t airspace_filter_cfg_size()
{
    return (&profile.airspace.display.undefined - &profile.airspace.display.below) + 1;
}

void airspace_filter_cfg_get(multi_value * out)
{
    uint8_t i = 0;
    for (cfg_entry_t * e = &profile.airspace.display.below; e <= &profile.airspace.display.undefined; e++)
    {
        out[i++] = e->value;
    }
}

airspace_record_t * airspace_load_cache(char * orig, uint16_t * loaded, uint16_t * hidden, uint32_t * mem_used)
{
    char name[REDCONF_NAME_MAX];
    char path[PATH_LEN];

    filemanager_get_filename(name, orig);

    snprintf(path, sizeof(path), "%s/%s", PATH_AIRSPACE_CACHE_DIR, name);

    int f = red_open(path, RED_O_RDONLY);

    *mem_used = 0;

    if (f > 0)
    {
        REDSTAT stat_rd;
        REDSTAT stat;

        uint32_t ver;
        red_read(f, &ver, sizeof(uint32_t));
        if (ver != CACHE_VERSION)
        {
            red_close(f);
            return NULL;
        }

        red_read(f, &stat_rd, sizeof(stat_rd));

        int32_t o = red_open(orig, RED_O_RDONLY);
        if (o > 0)
        {
            red_fstat(o, &stat);
            red_close(o);

            if (memcmp(&stat, &stat_rd, sizeof(stat)) != 0)
            {
                INFO("Cache file not valid");
                red_close(f);
                return NULL;
            }
        }
        else
        {
            red_close(f);
            return NULL;
        }

        uint8_t disp_cfg_num = airspace_filter_cfg_size();
        multi_value filter[disp_cfg_num];
        multi_value filter_rd[disp_cfg_num];
        airspace_filter_cfg_get(filter);

        red_read(f, filter_rd, disp_cfg_num * sizeof(multi_value));
        if (memcmp(filter, filter_rd, disp_cfg_num * sizeof(multi_value)) != 0)
        {
            INFO("Cache file not valid");
            red_close(f);
            return NULL;
        }

        red_read(f, loaded, sizeof(uint16_t));
        red_read(f, hidden, sizeof(uint16_t));

        airspace_record_t * first = NULL;
        airspace_record_t * actual;

        uint16_t cnt = *loaded - *hidden;

        for (uint16_t i = 0; i < cnt; i++)
        {
            if (first == NULL)
            {
                actual = ps_malloc(sizeof(airspace_record_t));
                first = actual;
            }
            else
            {
                airspace_record_t * new = ps_malloc(sizeof(airspace_record_t));
                actual->next = new;
                actual = new;
            }

            *mem_used += sizeof(airspace_record_t);

            red_read(f, actual, sizeof(airspace_record_t));
            actual->name = NULL;
            actual->points = NULL;
            actual->next = NULL;

            uint16_t name_len;
            red_read(f, &name_len, sizeof(uint16_t));
            if (name_len != 0)
            {
                actual->name = ps_malloc(name_len + 1);
                red_read(f, actual->name, name_len);
                actual->name[name_len] = 0;

                *mem_used += name_len + 1;
            }

            actual->points = ps_malloc(sizeof(gnss_pos_t) * actual->number_of_points);

            red_read(f, actual->points, sizeof(gnss_pos_t) * actual->number_of_points);

            *mem_used += sizeof(gnss_pos_t) * actual->number_of_points;
        }

        red_close(f);
        return first;
    }

    return NULL;
}

void airspace_save_cache(char * orig, airspace_record_t * first, uint16_t loaded, uint16_t hidden)
{
    char name[REDCONF_NAME_MAX];
    char path[PATH_LEN];

    filemanager_get_filename(name, orig);
    red_mkdir(PATH_AIRSPACE_CACHE_DIR);

    snprintf(path, sizeof(path), "%s/%s", PATH_AIRSPACE_CACHE_DIR, name);

    int f = red_open(path, RED_O_CREAT | RED_O_WRONLY | RED_O_TRUNC);

    if (f > 0)
    {
        int32_t o = red_open(orig, RED_O_RDONLY);
        if (o > 0)
        {
            REDSTAT stat;
            red_fstat(o, &stat);
            red_close(o);

            uint32_t ver = CACHE_VERSION;
            red_write(f, &ver, sizeof(uint32_t));
            red_write(f, &stat, sizeof(stat));
        }
        else
        {
            red_close(f);
            return;
        }

        uint8_t disp_cfg_num = airspace_filter_cfg_size();
        multi_value filter[disp_cfg_num];
        airspace_filter_cfg_get(filter);
        red_write(f, filter, disp_cfg_num * sizeof(multi_value));


        red_write(f, &loaded, sizeof(uint16_t));
        red_write(f, &hidden, sizeof(uint16_t));

        airspace_record_t * actual = first;

        while (actual != NULL)
        {
            red_write(f, actual, sizeof(airspace_record_t));

            uint16_t name_len = 0;
            if (actual->name != NULL)
            {
                name_len = strlen(actual->name);
                red_write(f, &name_len, sizeof(uint16_t));
                red_write(f, actual->name, name_len);
            }
            else
            {
                red_write(f, &name_len, sizeof(uint16_t));
            }

            red_write(f, actual->points, sizeof(gnss_pos_t) * actual->number_of_points);

            actual = actual->next;
        }

        red_close(f);
    }
}

void airspace_reload_parallel_task(void * param)
{
    osMutexAcquire(fc.airspaces.lock, WAIT_INF);

    if (strlen(config_get_text(&profile.airspace.filename)) > 0)
    {
        char path[PATH_LEN];
        snprintf(path, sizeof(path), PATH_AIRSPACE_DIR "/%s", config_get_text(&profile.airspace.filename));

        uint16_t loaded;
        uint16_t hidden;
        uint32_t mem_used;

        fc.airspaces.list = airspace_load(path, &loaded, &hidden, &mem_used, false);
        if (fc.airspaces.list != NULL)
        {
            fc.airspaces.valid = true;
            fc.airspaces.loaded = loaded;
            fc.airspaces.hidden = hidden;
            fc.airspaces.mem_used = mem_used;
        }
        else
        {
            fc.airspaces.valid = false;
            config_set_text(&profile.airspace.filename, "");
        }

        for (uint8_t i = 0; i < 9; i++)
        {
            gui.map.chunks[i].ready = false;
        }
    }

    osMutexRelease(fc.airspaces.lock);

    RedTaskUnregister();
    vTaskDelete(NULL);
}

void airspace_reload_parallel()
{
    xTaskCreate((TaskFunction_t)airspace_reload_parallel_task, "as_load_parallel_task", 1024 * 2, NULL, osPriorityIdle + 1, NULL);
}

void airspace_unload()
{
    fc.airspaces.valid = false;
    osMutexAcquire(fc.airspaces.lock, WAIT_INF);

    if (fc.airspaces.list != NULL)
    {

        airspace_free(fc.airspaces.list);
        fc.airspaces.list = NULL;
        fc.airspaces.loaded = 0;
        fc.airspaces.hidden = 0;
        fc.airspaces.mem_used = 0;

        for (uint8_t i = 0; i < 9; i++)
        {
            gui.map.chunks[i].ready = false;
        }

    }
    config_set_text(&profile.airspace.filename, "");

    osMutexRelease(fc.airspaces.lock);
}
