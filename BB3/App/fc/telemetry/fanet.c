/*
 * fanet.c
 *
 *  Created on: Oct 16, 2023
 *      Author: horinek
 */

#include "fc/fc.h"

bool fanet_msg(char * buff, uint16_t len)
{
    static uint16_t fanet_index = 0;

//    fc.fanet.neighbors_size = 2;'

    uint16_t fnt_cnt = 0;
    while (fnt_cnt < fc.fanet.neighbors_size)
    {
        fanet_index = (fanet_index + 1) % fc.fanet.neighbors_size;
        fnt_cnt++;

        neighbor_t * nb = &fc.fanet.neighbor[fanet_index];

//        if (fanet_index == 0)
//        {
//            nb->updated = 0xFF;
//            strcpy(nb->name, "test_fanet");
//            nb->flags = 1;
//            nb->addr.manufacturer_id = 0x11;
//            nb->addr.user_id = 0x1234;
//            nb->latitude = 48.143693 * GNSS_MUL;
//            nb->longitude = 17.077008 * GNSS_MUL;
//            nb->alititude = 100;
//            nb->climb = 1;
//            nb->speed = 5;
//            nb->heading = 5;
//        }
//
//        if (fanet_index == 1)
//        {
//            nb->updated = 0xFF;
//            strcpy(nb->name, "test_fanet2");
//            nb->flags = 1;
//            nb->addr.manufacturer_id = 0x11;
//            nb->addr.user_id = 0x1235;
//            nb->latitude = 48.143693 * GNSS_MUL;
//            nb->longitude = 17.08008 * GNSS_MUL;
//            nb->alititude = 200;
//            nb->climb = 2;
//            nb->speed = 10;
//            nb->heading = 0;
//        }



        if (nb->updated & NB_UPDATE_FWD)
        {
            nb->updated &= ~NB_UPDATE_FWD;

            uint8_t type = (nb->flags & NB_GROUND_TYPE_MASK) + ((nb->flags & NB_IS_FLYING == 0) ? 10 : 0);

            char tmp[128];

            snprintf(tmp, sizeof(tmp), "FNNGB,%02X,%04X,%s,%u,%0.5f,%0.5f,%d,%d,%d,%d",
                    nb->addr.manufacturer_id, nb->addr.user_id, nb->name,
                    type, nb->latitude / (float)GNSS_MUL, nb->longitude / (float)GNSS_MUL,
                    nb->alititude, nb->climb, nb->speed, nb->heading);

            snprintf(buff, len, "$%s*%02X\r\n", tmp, nmea_checksum(tmp));

            return true;
        }
    }

    return false;
}
