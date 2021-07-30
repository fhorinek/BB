/*
 * fanet.h
 *
 *  Created on: 27. 5. 2020
 *      Author: horinek
 */

#ifndef DRIVERS_FANET_H_
#define DRIVERS_FANET_H_

#include "../common.h"

void fanet_init();
void fanet_enable();
void fanet_disable();
void fanet_step();

void fanet_configure_flarm(bool init);
void fanet_configure_type(bool init);

#define FANET_AIRCRAFT_TYPE_OTHER				0
#define FANET_AIRCRAFT_TYPE_PARAGLIDER			1	//with FLARM
#define FANET_AIRCRAFT_TYPE_HANGGLIDER			2	//with FLARM
#define FANET_AIRCRAFT_TYPE_BALLON				3
#define FANET_AIRCRAFT_TYPE_GLIDER				4
#define FANET_AIRCRAFT_TYPE_POWERED				5
#define FANET_AIRCRAFT_TYPE_HELICOPTER			6
#define FANET_AIRCRAFT_TYPE_UAV					7

#define FANET_GROUND_TYPE_OTHER					0
#define FANET_GROUND_TYPE_WALKING				1
#define FANET_GROUND_TYPE_VEHICLE				2
#define FANET_GROUND_TYPE_BIKE					3
#define FANET_GROUND_TYPE_BOOT					4
#define FANET_GROUND_TYPE_NEED_RIDE				5
#define FANET_GROUND_TYPE_NEED_TECH_SUPP		12
#define FANET_GROUND_TYPE_NEED_MEDICAL			13
#define FANET_GROUND_TYPE_DISTRESS				14
#define FANET_GROUND_TYPE_DISTRESS_AUTO			15


#define FANET_MSG_TYPE_ACK						0
#define FANET_MSG_TYPE_TRACKING					1
#define FANET_MSG_TYPE_NAME						2
#define FANET_MSG_TYPE_MESSAGE					3
#define FANET_MSG_TYPE_SERVICE					4
#define FANET_MSG_TYPE_LANDMARKS				5
#define FANET_MSG_TYPE_REMOTE_CONFIGURATION		6
#define FANET_MSG_TYPE_GROUND_TRACKING			7

#define FANET_TX_TRACKING_PERIOD				(1000)
#define FANET_TX_NAME_PERIOD					(1000 * 60 * 4)

#define FANET_ADDR_MULTICAST					0x0000

#endif /* DRIVERS_FANET_H_ */
