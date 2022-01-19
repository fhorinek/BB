/*
 * wind2.cpp
 *
 *  Created on: Jan 23, 2017
 *      Author: fiala
 */

#define DEBUG_LEVEL DBG_DEBUG

#include "wind.h"
#include "fc.h"
#include "../common.h"

#define SECTOR_CNT  16
#define SAMPLES     100

void wind_new_fix()
{
    struct
    {
        int32_t spd_acc;
        int32_t hdg_acc;
        float spd;
        float hdg;

        uint16_t cnt;
        uint8_t _pad[2];
    } sectors [SECTOR_CNT] = {0};

    //get the samples
    uint8_t step = 1000 / FC_HISTORY_PERIOD;
    uint16_t max_samples = SAMPLES;
    for (uint16_t i = 0; i < FC_HISTORY_SIZE; i++)
    {
        uint16_t index = (fc.history.index + FC_HISTORY_SIZE - i * step) % FC_HISTORY_SIZE;

        if (fc.history.positions[index].flags & (FC_POS_GNSS_2D | FC_POS_GNSS_3D))
        {
            uint8_t sector = fc.history.positions[index].ground_hdg / (360 / SECTOR_CNT);

            sectors[sector].spd_acc += fc.history.positions[index].ground_spd / 100;
            sectors[sector].hdg_acc += fc.history.positions[index].ground_hdg;
            sectors[sector].cnt++;
        }

        max_samples--;
        if (max_samples == 0)
            break;
    }

    //calc average
    for (uint8_t i = 0; i < SECTOR_CNT; i++)
    {
        if (sectors[i].cnt > 0)
        {
            sectors[i].hdg = sectors[i].hdg_acc / sectors[i].cnt;
            sectors[i].spd = sectors[i].spd_acc / sectors[i].cnt;
        }
        else
        {
            //we have not enough data to compute the wind
            //end the calculation
            return;
        }
    }

    float wind_x_acc = 0;
    float wind_y_acc = 0;
    //calc diff and vectors
    for (uint8_t i = 0; i < SECTOR_CNT; i++)
    {
        float diff = sectors[(i + SECTOR_CNT / 2) % SECTOR_CNT].spd - sectors[i].spd;

        float angle = to_radians(90 + 180 + sectors[i].hdg);
        wind_x_acc += cos(angle) * diff;
        wind_y_acc += sin(angle) * diff;
    }

    float wind_x = wind_x_acc / SECTOR_CNT;
    float wind_y = wind_y_acc / SECTOR_CNT;

    fc.wind.valid = true;
    fc.wind.direction = ((int16_t)to_degrees(atan2(wind_x, -wind_y)) + 360) % 360;
    fc.wind.speed = sqrt(pow(wind_x, 2) + pow(wind_y, 2));
}

void wind_init()
{
    fc.wind.valid = false;

    DBG("wind_init\n");
}

void wind_step()
{
    if (fc.gnss.new_sample & FC_GNSS_NEW_SAMPLE_WIND)
    {
        wind_new_fix();

        fc.gnss.new_sample &= ~FC_GNSS_NEW_SAMPLE_WIND;
    }
}
