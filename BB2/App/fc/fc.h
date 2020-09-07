/*
 * fc.h
 *
 *  Created on: May 6, 2020
 *      Author: horinek
 */

#ifndef FC_FC_H_
#define FC_FC_H_

#include "../common.h"

//metric to imperial
#define FC_METER_TO_FEET		(3.2808399)
#define FC_MPS_TO_100FPM		(1.96850394)  	//100 feet per min (WTF?)

#define FC_KNOTS_TO_KPH		(1.852)				//Kilometers per hour
#define FC_KNOTS_TO_MPH		(1.15077945)		//Miles per hour
#define FC_KNOTS_TO_MPS		(0.51444444444)		//Meters per seconds
#define FC_MPS_TO_KPH		(3.6)				//Kilometers per hour
#define FC_MPS_TO_MPH		(2.23693629)		//Miles per hour
#define FC_MPS_TO_KNOTS		(1.94384449)		//Knots
#define FC_KM_TO_MILE		(0.621371)

;

#define GNSS_NUMBER_OF_SATS		12

#define GNSS_GPS				0
#define GNSS_GLONAS				1
#define GNSS_GALILEO			2

#define GNSS_NUMBER_OF_SYSTEMS	3
#define GNSS_MUL				10000000l



typedef struct
{
	uint8_t manufacturer_id;
	uint16_t user_id;
} fanet_addr_t;

#define NB_NUMBER_IN_MEMORY		50

//neighbor flag
//when flying
#define NB_AIRCRAFT_TYPE_MASK	0b00000111
#define NB_HAVE_TURNRATE		0b00001000

//when walking
#define NB_GROUND_TYPE_MASK		0b00001111

#define NB_ONLINE_TRACKING		0b00010000
#define NB_FRIEND				0b00100000
#define NB_IS_FLYING			0b01000000

#define NB_HAVE_POS				0b10000000

#define NB_NAME_LEN				16
#define NB_TOO_FAR				0xFFFF

typedef struct
{
	fanet_addr_t addr;
	char name[NB_NAME_LEN];

	int32_t latitude;
	int32_t longitude;
	uint16_t alititude;

	uint8_t flags;
	uint8_t heading;

	uint16_t timestamp;
	uint16_t dist; //in m
} neighbor_t;


typedef struct
{
	struct
	{
		bool valid;
		uint8_t fix;
		uint8_t first_fix;
		uint32_t fix_time;

		int32_t latitude;   //*10^7
		int32_t longtitude; //*10^7

		float ground_speed;
		uint16_t heading;

		uint32_t utc_time;

		float altitude;
		float geoid_separation;

		struct
		{
			uint8_t fix;

			float pdop; //Position Dilution of Precision
			float hdop; //Horizontal Dilution of Precision
			float vdop; //Vertical Dilution of Precision

			uint8_t sat_total;
			uint8_t sat_used;

			struct
			{
				uint8_t sat_id;
				int8_t elevation; // +/- 90
				uint8_t azimuth; //0-359 /2
				uint8_t snr;
			} sats[GNSS_NUMBER_OF_SATS];

		} sat_info[GNSS_NUMBER_OF_SYSTEMS];
	} gnss;

	struct
	{
		bool valid;

		char version[20];
		fanet_addr_t addr;

		neighbor_t neighbor[NB_NUMBER_IN_MEMORY];
		uint8_t neighbors_size;
		uint8_t neighbors_magic;
	} fanet;
} fc_t;

extern fc_t fc;

#endif /* FC_FC_H_ */
