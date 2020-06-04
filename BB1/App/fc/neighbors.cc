
#include "neighbors.h"

#include "fc.h"
#include "../config/config.h"
#include "../etc/gnss_calc.h"

bool operator==(const fanet_addr_t& lhs, const fanet_addr_t& rhs)
{
    return lhs.manufacturer_id == rhs.manufacturer_id && lhs.user_id == rhs.user_id;
}

void neighbors_init()
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
		if (fc.fanet.neighbor[i].addr == addr)
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
	neighbor_t * nb = &fc.fanet.neighbor[fc.fanet.neighbors_size];
	fc.fanet.neighbors_size += 1;

	nb->addr.manufacturer_id = addr.manufacturer_id;
	nb->addr.user_id = addr.user_id;

	nb->flags = 0;
	nb->name[0] = 0;

	return nb;
}

void neighbors_update_distance(neighbor_t * nb)
{
	if (!(nb->flags & NB_HAVE_POS))
		return;

	bool FAI = config_get_select(&config.settings.units.earth_model) == EARTH_FAI;
	uint32_t dist = gnss_distance(nb->latitude, nb->longitude, fc.gnss.latitude, fc.gnss.longtitude, FAI, NULL);

	nb->dist = min(NB_TOO_FAR, dist);
}

void neighbors_update(neighbor_t new_data)
{
	neighbor_t * nb = neighbors_find(new_data.addr);

	//not found add new
	if (nb == NULL)
		nb = neighbors_add(new_data.addr);

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

void neighbors_update_name(fanet_addr_t addr, char * name)
{
	neighbor_t * nb = neighbors_find(addr);

	//not found add new
	if (nb == NULL)
		nb = neighbors_add(addr);

	memcpy(&nb->name, name, NB_NAME_LEN);
	nb->name[NB_NAME_LEN - 1] = 0;

	//notify GUI
	neighbors_update_magic();
}

void neighbors_step()
{
	static uint32_t next_update = 0;

	if (next_update < HAL_GetTick())
	{
		next_update = HAL_GetTick() + 1000;

		//update distance
		for (uint8_t i = 0; i < fc.fanet.neighbors_size; i++)
		{
			neighbor_t * nb = &fc.fanet.neighbor[i];

			//distance not updated in 1 s
			if (HAL_GetTick() - nb->timestamp > 1000)
				neighbors_update_distance(nb);

		}

		//notify GUI
		neighbors_update_magic();
	}


}
