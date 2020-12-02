/*
 * fc.h
 *
 *  Created on: May 6, 2020
 *      Author: horinek
 */

#ifndef FC_FC_H_
#define FC_FC_H_

#include "../common.h"

//unit conversions
#define FC_METER_TO_FEET		(3.2808399)
#define FC_MPS_TO_100FPM		(1.96850394)  	//100 feet per min (WTF?)

#define FC_KNOTS_TO_KPH		(1.852)				//Kilometers per hour
#define FC_KNOTS_TO_MPH		(1.15077945)		//Miles per hour
#define FC_KNOTS_TO_MPS		(0.51444444444)		//Meters per seconds
#define FC_MPS_TO_KPH		(3.6)				//Kilometers per hour
#define FC_MPS_TO_MPH		(2.23693629)		//Miles per hour
#define FC_MPS_TO_KNOTS		(1.94384449)		//Knots
#define FC_KM_TO_MILE		(0.621371)


#define GNSS_NUMBER_OF_SATS		32

#define GNSS_GPS				0
#define GNSS_GLONAS				1
#define GNSS_GALILEO			2

#define GNSS_MUL				10000000l

#define GNSS_SAT_SYSTEM_MASK	0b00000111
#define GNSS_SAT_GPS			0b00000000
#define GNSS_SAT_SBAS			0b00000001
#define GNSS_SAT_GALILEO		0b00000010
#define GNSS_SAT_BEIDOU			0b00000011
#define GNSS_SAT_IMES			0b00000100
#define GNSS_SAT_QZSS			0b00000101
#define GNSS_SAT_GLONASS		0b00000110

#define GNSS_SAT_USED			0b00001000

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

	uint16_t max_dist;
} neighbor_t;


typedef struct
{
	struct
	{
		bool valid;
		uint8_t fix; //2 - 2D, 3 - 3D
		bool time_synced;

		uint32_t ttf; //[ms]

		int32_t latitude;   //*10^7
		int32_t longtitude; //*10^7

		float ground_speed; //[m/s]
		uint16_t heading; //deg

		uint32_t utc_time;

		float altitude_above_ellipsiod; //[m]
		float altitude_above_msl; //[m]

		uint16_t horizontal_accuracy; //[m]
		uint16_t vertical_accuracy; //[m]

		struct
		{
			uint8_t sat_total;
			uint8_t sat_used;

			struct
			{
				uint8_t sat_id;
				int8_t elevation; // +/- 90
				uint8_t azimuth; //0-359 /2
				uint8_t snr;
				uint8_t flags;
			} sats[GNSS_NUMBER_OF_SATS];
		} sat_info;
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

	struct
	{
		bool valid;

		float pressure; //in Pa
	} vario;
} fc_t;

extern fc_t fc;

void fc_set_time_from_utc(uint32_t datetime);

#endif /* FC_FC_H_ */
