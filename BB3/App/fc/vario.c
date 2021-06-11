/*
 * vario.c
 *
 *  Created on: Feb 5, 2021
 *      Author: horinek
 */
#include "vario.h"

#include "fc/fc.h"
#include "fc/kalman.h"

#include "drivers/esp/protocol.h"

void vario_init()
{
    fc.fused.status = fc_dev_init;
    fc.fused.vario = 0;
}

typedef struct
{
    int16_t climb;
    int16_t freq;
    int16_t dura;
} vario_tone_point_t;

typedef struct
{
    vario_tone_point_t * points;
    uint32_t size;
} vario_tone_t;

typedef struct
{
    vario_tone_t * tones;
    uint32_t size;
} vario_profile_t;

vario_profile_t * current_profile = NULL;

void vario_profile_free(vario_profile_t * prof)
{
    if (prof != NULL)
    {
        for (uint8_t i = 0; i < prof->size; i++)
        {
            if (prof->tones[i].size > 0)
                ps_free(prof->tones[i].points);

        }
        if (prof->size > 0)
            ps_free(prof->tones);

        ps_free(prof);
    }
}

void vario_profile_load(char * name)
{
    char path[PATH_LEN];
    snprintf(path, sizeof(path), "%s/%s.cfg", PATH_VARIO_DIR, name);

    if (!file_exists(path))
    {
    	char def[PATH_LEN];
    	snprintf(def, sizeof(def), "%s/defaults/vario/default.cfg", PATH_ASSET_DIR);
    	copy_file(def, path);
    }

    vario_profile_t * prof = NULL;
    int16_t tone_size;

    bool ret = db_query_int(path, "tone_size", &tone_size);
    if (ret && tone_size > 0 && tone_size <= 8)
    {
        prof = ps_malloc(sizeof(vario_profile_t));
        prof->size = tone_size;
        prof->tones = ps_malloc(sizeof(vario_tone_t) * tone_size);

        for (uint8_t tone = 0; tone < tone_size; tone++)
        {
            char key[16];
            snprintf(key, sizeof(key), "tone_%u_size", tone);

            int16_t cnt;
            ret = db_query_int(path, key, &cnt);
            if (ret)
            {
                prof->tones[tone].size = cnt;
                prof->tones[tone].points = ps_malloc(sizeof(vario_tone_point_t) * cnt);

                for (uint8_t i = 0; i < cnt; i++)
                {
                    ret = 1;
                    snprintf(key, sizeof(key), "tone_%u_%u_c", tone, i);
                    ret &= db_query_int(path, key, &prof->tones[tone].points[i].climb);
                    snprintf(key, sizeof(key), "tone_%u_%u_f", tone, i);
                    ret &= db_query_int(path, key, &prof->tones[tone].points[i].freq);
                    snprintf(key, sizeof(key), "tone_%u_%u_d", tone, i);
                    ret &= db_query_int(path, key, &prof->tones[tone].points[i].dura);

                    if (!ret)
                        break;
                }
            }
            else
            {
                break;
            }

            if (!ret)
                break;
        }


    }

    if (!ret)
    {
        vario_profile_free(prof);
        //load default
        ASSERT(0);
    }
    else
    {
        vario_profile_free(current_profile);
        current_profile = prof;
    }

}

bool get_between(vario_tone_t * tone, int16_t climb, int16_t * freq, int16_t * dura)
{
    uint8_t prev = 0xFF;
    uint8_t next = 0xFF;

    for (uint8_t i = 0; i < tone->size; i++)
    {
        if (tone->points[i].climb <= climb)
        {
            prev = i;
            continue;
        }

        if (tone->points[i].climb >= climb)
        {
            next = i;
            break;
        }
    }

    if (prev != 0xFF && next != 0xFF)
    {
        float m = (climb - tone->points[prev].climb) / (float)(tone->points[next].climb - tone->points[prev].climb);
        *freq = tone->points[prev].freq + m * (tone->points[next].freq - tone->points[prev].freq);
        *dura = tone->points[prev].dura + m * (tone->points[next].dura - tone->points[prev].dura);

        return true;
    }

    return false;
}

static bool vario_is_silent = true;

void vario_play_tone(float vario)
{
    static int16_t last_value = 0xFFFF;
    int16_t vario_int = vario * 10;
    static uint32_t next_time = 0;

    if (current_profile == NULL)
        return;

    if (next_time < HAL_GetTick())
    {
        next_time = HAL_GetTick() + 50;
    }
    else
    {
        return;
    }


    if (vario_int != last_value || vario_is_silent)
    {
        last_value = vario_int;
        vario_is_silent = false;

        proto_tone_play_t data;
        data.size = 0;
        for (uint8_t i = 0; i < current_profile->size; i++)
        {
            int16_t freq, dura;

            if (get_between(&current_profile->tones[i], vario_int, &freq, &dura))
            {
                data.freq[data.size] = freq;
                data.dura[data.size] = dura;
                data.size++;
            }
        }

        protocol_send(PROTO_TONE_PLAY, (void *)&data, sizeof(data));
    }

}

void vario_silent()
{
	if (vario_is_silent)
		return;

	vario_is_silent = true;

    proto_tone_play_t data;
    data.size = 0;
    protocol_send(PROTO_TONE_PLAY, (void *)&data, sizeof(data));
}

void vario_step()
{
    static uint16_t skip = 1000;

    if (fc.baro.status != fc_dev_ready)
        return;

    float raw_altitude = fc_press_to_alt(fc.baro.pressure, config_get_big_int(&config.vario.qnh1));

    if (isnan(raw_altitude))
        return;

    if (fc.fused.status == fc_dev_init)
    {
        fc.fused.status = fc_dev_sampling;
        kalman_configure(raw_altitude);
    }

    float altitude, vario;
    float acc = fc.imu.acc_gravity_compensated * config_get_float(&profile.vario.acc_gain);

    kalman_step(raw_altitude, acc, &altitude, &vario);

    if (isnan(altitude) || isnan(vario))
        return;


    if (fc.fused.status == fc_dev_sampling)
    {
        skip--;
        if (skip == 0)
        {
            fc.fused.status = fc_dev_ready;
            fc.fused.avg_vario = vario;
            fc.fused.altitude1 = altitude;
            fc_reset();
        }
        else
        {
            return;
        }
    }

    fc.fused.vario = vario;
    fc.fused.avg_vario += (vario - fc.fused.avg_vario) / (float)(config_get_int(&profile.vario.avg_duration) * 100);
    fc.fused.altitude1 = altitude;
    fc.fused.pressure = fc_alt_to_press(altitude, config_get_big_int(&config.vario.qnh1));
    fc.fused.altitude2 = fc_press_to_alt(fc.fused.pressure, config_get_big_int(&config.vario.qnh2));

    int16_t ivario = vario * 10;
    if (config_get_bool(&profile.vario.in_flight) && fc.flight.mode != flight_flight)
    {
    	vario_silent();
    }
    else
    {
		if (config_get_int(&profile.vario.sink) >= ivario
			 || config_get_int(&profile.vario.lift) <= ivario)
		{
			vario_play_tone(vario);
		}
		else
		{
			vario_silent();
		}
    }
}
