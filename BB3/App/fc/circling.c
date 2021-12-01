
#include "circling.h"
#include "fc.h"

void circling_reset()
{
    fc.flight.total_heading_change = 0;
    fc.flight.avg_heading_change = 0;
    fc.flight.last_heading = 0;

    fc.flight.circling = false;
    fc.flight.circling_stop = 0;
    fc.flight.circling_time = 0;
    fc.flight.circling_gain = 0;

	for (uint8_t i = 0; i < 8; i++)
		fc.flight.circling_history[i] = 0;
}

void circling_step()
{
	if (fc.fused.status == fc_dev_ready && fc.gnss.fix == 3)
	{
		uint8_t cindex = ((((uint16_t)fc.gnss.heading + 360 - 22) / 45) + 1) % 8;

		float val = fc.fused.vario * VARIO_CIRCLING_HISTORY_SCALE;
		if (val > 127) val = 127;
		if (val < -126) val = -126;

//		DBG("ch %u %u %i\n", fc.gps_data.heading, cindex, int8_t(val));
		fc.flight.circling_history[cindex] = (int8_t)val;
	}

	if (fc.gnss.new_sample & FC_GNSS_NEW_SAMPLE_CIRCLE)
	{
        fc.gnss.new_sample &= ~FC_GNSS_NEW_SAMPLE_CIRCLE;

	    if (((abs(fc.flight.total_heading_change) > 360 && !fc.flight.circling) ||
             (abs(fc.flight.total_heading_change) > 340 &&  fc.flight.circling)) &&
             (abs(fc.flight.avg_heading_change) > PAGE_AUTOSET_CIRCLING_THOLD))
        {
            if (!fc.flight.circling)
            {
//                gui_page_set_mode(PAGE_MODE_CIRCLING);
                fc.flight.circling = true;
                fc.flight.circling_start = HAL_GetTick();
                fc.flight.circling_start_altitude = fc.fused.altitude1;
            }

            fc.flight.circling_stop = 0;
            fc.flight.circling_time = (HAL_GetTick() - fc.flight.circling_start) / 1000;
            fc.flight.circling_gain = fc.fused.altitude1 - fc.flight.circling_start_altitude;
        }
        else
        {
            if (fc.flight.circling)
            {
                if (fc.flight.circling_stop == 0)
                    fc.flight.circling_stop = HAL_GetTick();

                if (HAL_GetTick() - fc.flight.circling_stop > (config_get_int(&profile.flight.circle_timeout) * 1000))
                {
                    fc.flight.circling = false;
                    fc.flight.total_heading_change = 0;
//                    gui_page_set_mode(PAGE_MODE_NORMAL);
                }
            }
        }

	    int16_t heading_change = fc.gnss.heading - fc.flight.last_heading;
	    if (heading_change > 180)
	    	heading_change -= 360;
	    if (heading_change < -180)
	    	heading_change += 360;

        fc.flight.last_heading = fc.gnss.heading;

//        DBG("\nhch %d\n", heading_change);


        fc.flight.avg_heading_change += (heading_change - fc.flight.avg_heading_change) / PAGE_AUTOSET_CIRCLING_AVG;
        fc.flight.total_heading_change += heading_change;

        //if avg have different sign than total zero the total
        if (fc.flight.avg_heading_change > 0 && fc.flight.total_heading_change < 0)
        	fc.flight.total_heading_change = 0;
        else if (fc.flight.avg_heading_change < 0 && fc.flight.total_heading_change > 0)
        	fc.flight.total_heading_change = 0;

        fc.flight.total_heading_change = CLAMP(fc.flight.total_heading_change, -400, 400);

//        DBG("ach %0.2f\n", fc.flight.avg_heading_change);
//        DBG("tch %d\n", fc.flight.total_heading_change);
//        DBG("cr %d\n", fc.flight.circling);
	}
}
