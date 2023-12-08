/*
 * fanet.c
 *
 *  Created on: Nov 18, 2023
 *      Author: kolupaev
 */


#include "fc/fc.h"
#include "drivers/power/pwr_mng.h"

#include "etc/epoch.h"
#include "etc/format.h"

static int8_t last_transmitted_index = -1;
static uint8_t magic_number = 0;
static uint16_t update_start_update_ts = 0;
static uint16_t last_update_ts = 0;

/**
 * Creates signed FNNGB message representing a single neighbor
 * Subsequent invocations will evaluate neighbors in round-robin manner
 */
bool fanet_forwarding_msg(char * buf, uint16_t len)
{

	/*
	 * https://gitlab.com/xcontest-public/xctrack-public/-/issues/380
	 * $FNNGB,manufacturer(hex),id(hex),name(up to 32bytes),type/status,latitude,longitude,altitude,climb,speed,heading*checksum
	 * manufacturer:	1-2 chars hex
	 * id:			1-4 chars hex
	 * name:		string up to 32 chars
	 * type/status:		while airborne: aircraft type: 0-7 (3D tracking), else: status: 0-15 (2D tracking) +10 -> 10-25
	 * latitude:		%.5f in degree
	 * longitude:		%.5f in degree
	 * altitude:		%.f in meter, -1000 for ground
	 * climb:		%.1f in m/s
	 * speed:		%.1f in km/h
	 * heading:		%.f in degree
	 *
	 * for the types please see: https://github.com/3s1d/fanet-stm32/blob/master/Src/fanet/radio/protocol.txt
	 *
	 */

	if(fc.fanet.neighbors_magic == magic_number || fc.fanet.neighbors_size == 0) {
		return false;
	}

	neighbor_t * neighbor;

	do {
		last_transmitted_index = (last_transmitted_index + 1) % fc.fanet.neighbors_size;
		neighbor = &fc.fanet.neighbor[last_transmitted_index];
	} while (!(last_transmitted_index == 0 || neighbor->timestamp > last_update_ts));

	if(last_transmitted_index == 0) { // At the head of the list, note start time to avoid race condition
		update_start_update_ts = HAL_GetTick() / 1000;

	}

	int res = snprintf(buf, len, "$FNNGB,%x,%x,%s,%d,%.5f,%.5f,%d,%.1f,%.1f,%.1f",
		neighbor->addr.manufacturer_id, neighbor->addr.user_id, neighbor->name,
		neighbor->flags & NB_AIRCRAFT_TYPE_MASK,
		neighbor->latitude / (float)GNSS_MUL, neighbor-> longitude / (float)GNSS_MUL,
		neighbor->alititude,
		neighbor->climb_rate / 10.0f, neighbor->speed / 10.0f, neighbor->heading * 1.412f // 360/255
		);

	if(res <= 0) {
		WARN("Unable to construct FNNGB message");
		return false;
	}

	res = snprintf(&buf[res], len - res, "*%02X\r\n", nmea_checksum(&buf[1]));

	if(res <= 0) {
		WARN("Unable to sign FNNGB message");
		return false;
	}

	// We made a full loop sending out neighbors
	if(last_transmitted_index == fc.fanet.neighbors_size - 1) {
	 	last_update_ts = update_start_update_ts;
	 	magic_number = fc.fanet.neighbors_magic;
	}

	return true;
}
