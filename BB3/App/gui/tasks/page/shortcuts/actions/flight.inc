/*
 * dummy.h
 *
 *  Created on: 11. 10. 2022
 *      Author: horinek
 */


static bool shrt_flight_start_action()
{
    if (fc.flight.mode == flight_wait_to_takeoff || fc.flight.mode == flight_landed)
    {
        pages_popup("Flight start");
        fc_takeoff();
    }
    else if (fc.flight.mode == flight_flight)
    {
        pages_popup("Flight stop");
        fc_landing();
    }

    return false;
}

static bool shrt_flight_start_icon(char * icon, char * label)
{
    if (fc.flight.mode == flight_flight)
        strcpy(icon, MD_STOP_CIRCLE);
    else
        strcpy(icon, MD_PLAY_CIRCLE);

    strcpy(label, "Start/Stop flight");

    return true;
}


