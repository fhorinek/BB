
#include <etc/geo_calc.h>
#include "neighbors.h"

#include "fc.h"

void neighbors_reset()
{
	for (uint8_t i = 0; i < NB_NUMBER_IN_MEMORY; i++)
	{
		fc.fanet.neighbor[i].flags = 0;
	}
	fc.fanet.neighbors_size = 0;
	fc.fanet.neighbors_magic = 0;
}

neighbor_t * neighbors_find(fanet_addr_t addr)
{
	//is in memory?
	for (uint8_t i = 0; i < fc.fanet.neighbors_size; i++)
	{
		if (fc.fanet.neighbor[i].addr.manufacturer_id == addr.manufacturer_id
			&& fc.fanet.neighbor[i].addr.user_id == addr.user_id)
		{
			return &fc.fanet.neighbor[i];
		}
	}
	return NULL;
}

void neighbors_update_magic()
{
	fc.fanet.neighbors_magic++;
}

neighbor_t * neighbors_add(fanet_addr_t addr)
{

	if (fc.fanet.neighbors_size >= NB_NUMBER_IN_MEMORY) {
		WARN("Max number of FANET NB reached!");
		return NULL;
	}

	neighbor_t * nb = &fc.fanet.neighbor[fc.fanet.neighbors_size];
	fc.fanet.neighbors_size += 1;

	nb->addr.manufacturer_id = addr.manufacturer_id;
	nb->addr.user_id = addr.user_id;

	nb->flags = 0;
	nb->name[0] = 0;

	nb->max_dist = 0;
	nb->dist = 0;

	return nb;
}

void neighbors_update_distance(neighbor_t * nb)
{
	if (!(nb->flags & NB_HAVE_POS))
		return;

	bool FAI = config_get_select(&config.units.earth_model) == EARTH_FAI;
	uint32_t dist = geo_distance(nb->latitude, nb->longitude, fc.gnss.latitude, fc.gnss.longtitude, FAI, NULL) / 100; // cm to m

	nb->dist = min(NB_TOO_FAR, dist);

	if (nb->dist != NB_TOO_FAR && nb->dist > nb->max_dist)
		nb->max_dist = nb->dist;
}

void neighbors_update(neighbor_t new_data)
{
	neighbor_t * nb = neighbors_find(new_data.addr);

	//not found add new
	if (nb == NULL)
		nb = neighbors_add(new_data.addr);

	if (nb != NULL) {
		nb->latitude = new_data.latitude;
		nb->longitude = new_data.longitude;
		nb->alititude = new_data.alititude;
		nb->heading = new_data.heading;
		nb->flags = (nb->flags & 0xF0) | (new_data.flags & 0x0F);
		nb->flags |= NB_HAVE_POS;
		nb->timestamp = HAL_GetTick() / 1000;

		neighbors_update_distance(nb);

		//notify GUI
		neighbors_update_magic();
	}
}

void neighbors_update_name(fanet_addr_t addr, char * name)
{
	neighbor_t * nb = neighbors_find(addr);

	//not found add new
	if (nb == NULL)
		nb = neighbors_add(addr);

	if (nb != NULL) {
		strncpy(nb->name, name, NB_NAME_LEN);
		nb->timestamp = HAL_GetTick() / 1000;

		//notify GUI
		neighbors_update_magic();
	}
}

// should be atomic?
void neighbors_remove_old() {
	for (uint8_t i = 0; i < fc.fanet.neighbors_size; i++)
	{
		uint32_t last_update = (HAL_GetTick() / 1000) - fc.fanet.neighbor[i].timestamp;
		if (last_update > NB_TOO_OLD) {
			if (i != (fc.fanet.neighbors_size - 1)) { // not last?
				for (uint8_t j = i; j < fc.fanet.neighbors_size; j++) {
					fc.fanet.neighbor[j] = fc.fanet.neighbor[j + 1];
				}
			}
			fc.fanet.neighbors_size--;
		}
	}

	neighbors_update_magic();
}

void neighbors_step()
{
	static uint32_t next_update = 0;
	static uint32_t remove_old = 0;

	if (next_update < HAL_GetTick())
	{
		next_update = HAL_GetTick() + 1000;

		//update distance
		for (uint8_t i = 0; i < fc.fanet.neighbors_size; i++)
		{
			neighbor_t * nb = &fc.fanet.neighbor[i];

			//last update in >1s update between position updates
            //XXX only for testing:
			//last update > 10 s we don't have active tracking, do not update
			uint32_t last_update = HAL_GetTick() - nb->timestamp;
			if (last_update > 1000 && last_update < 10000)
				neighbors_update_distance(nb);
		}

		//notify GUI
		neighbors_update_magic();
	}


	if (remove_old < HAL_GetTick()) {

		remove_old = HAL_GetTick() + NB_TOO_OLD * 1000;

		neighbors_remove_old();
	}
}
