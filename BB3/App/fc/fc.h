/*
 * fc.h
 *
 *  Created on: May 6, 2020
 *      Author: horinek
 */

#ifndef FC_FC_H_
#define FC_FC_H_

#include "common.h"
#include "drivers/nvm.h"

//unit conversions
#define FC_METER_TO_FEET		(3.2808399)
#define FC_MPS_TO_100FPM        (1.96850394)    //100 feet per min (WTF?)


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

#define NB_NAME_LEN				19
#define NB_TOO_FAR				0xFFFF

typedef struct
{
    int32_t latitude;
    int32_t longitude;

    uint16_t alititude;
    uint16_t timestamp;
    uint16_t dist; //in m
    uint16_t max_dist;

    fanet_addr_t addr;
    uint8_t flags;

    uint8_t heading;
	char name[NB_NAME_LEN];

} neighbor_t;

typedef enum
{
	esp_off = 0,
	esp_starting,
	esp_normal,
	esp_programming,
	esp_external
} esp_mode_t;

typedef enum
{
    fc_dev_error = 0,
    fc_dev_init,
    fc_dev_sampling,
    fc_dev_ready,
    fc_device_not_calibrated,
    fc_dev_off,
} fc_device_status_t;

typedef struct
{
	struct
	{
        uint32_t ttf; //[ms]

        int32_t latitude;   //*10^7
        int32_t longtitude; //*10^7
        float ground_speed; //[m/s]
        float heading;
        uint32_t utc_time;
        float altitude_above_ellipsiod; //[m]
        float altitude_above_msl; //[m]
        float horizontal_accuracy; //[m]
        float vertical_accuracy; //[m]

        fc_device_status_t status;
		uint8_t fix; //2 - 2D, 3 - 3D
		bool time_synced;
		uint8_t _pad[1];

		struct
		{
			uint8_t sat_total;
			uint8_t sat_used;
			uint8_t _pad[2];

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
        neighbor_t neighbor[NB_NUMBER_IN_MEMORY];
        fanet_addr_t addr;

        fc_device_status_t status;

		uint8_t neighbors_size;
		uint8_t neighbors_magic;

        char version[21];

	} fanet;

	struct
	{
        float pressure; //in Pa
        fc_device_status_t status;

        uint8_t _pad[3];
	} baro;

	struct
	{
        uint32_t version;

        fc_device_status_t status;
		esp_mode_t mode;
		uint8_t progress;
		uint8_t _pad[1];
	} esp;

	struct
	{
	    imu_calibration_t calibration;
        vector_float_t acc;
        vector_float_t gyro;
        vector_float_t mag;
        quaternion_t quat;
        float acc_total;
        float acc_gravity_compensated;

        struct
        {
            vector_i16_t acc;
            vector_i16_t gyro;
            vector_i16_t mag;
        } raw;
	    fc_device_status_t status;
	    uint8_t _pad[1];
	} imu;

	struct
	{
        float altitude;
        float pressure;
        float vario;
        float avg_vario;

        fc_device_status_t status;
        uint8_t _pad[3];

	    //history
	} fused;
} fc_t;

extern fc_t fc;

void fc_device_status(char * buff, fc_device_status_t status);

void fc_set_time_from_utc(uint32_t datetime);

void fc_init();

float fc_alt_to_qnh(float alt, float pressure);
float fc_press_to_alt(float pressure, float qnh);
float fc_alt_to_press(float alt, float qnh);


#endif /* FC_FC_H_ */
