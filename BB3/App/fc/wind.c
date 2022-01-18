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

#define SECTOR_CNT  8

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

    for (uint16_t i = 0; i < FC_HISTORY_SIZE; i++)
    {
        uint16_t index = (fc.history.index + FC_HISTORY_SIZE - i) % FC_HISTORY_SIZE;

        if (fc.history.positions[index].flags & (FC_POS_GNSS_2D | FC_POS_GNSS_3D))
        {
            uint8_t sector = fc.history.positions[index].ground_hdg / (360 / SECTOR_CNT);

            sectors[sector].spd_acc += fc.history.positions[index].ground_spd;
            sectors[sector].hdg_acc += fc.history.positions[index].ground_hdg;
            sectors[sector].cnt++;
        }
    }

    DBG("---wind---");
    for (uint8_t i = 0; i < SECTOR_CNT; i++)
    {
        if (sectors[i].cnt != 0)
        {
            sectors[i].spd = sectors[i].spd_acc / (100 * sectors[i].cnt);
            sectors[i].hdg = sectors[i].hdg_acc / sectors[i].cnt;
        }

        DBG(" %u %0.1f %0.1f %u", i, sectors[i].spd, sectors[i].hdg, sectors[i].cnt);
    }

}

void wind_init()
{
//    fc.wind.valid = false;

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
