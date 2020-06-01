
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
		if (fc.fanet.neighbor[i].flags & NB_VALID)
		{
			//addres found and entry is valid
			if (fc.fanet.neighbor[i].addr == addr)
			{
				return &fc.fanet.neighbor[i];
			}
		}
	}
	return NULL;
}

void neighbors_update_magic()
{
	fc.fanet.neighbors_magic++;
}

void neighbors_update(neighbor_t new_data)
{
	neighbor_t * nb = neighbors_find(new_data.addr);

	//not found add new
	if (nb == NULL)
	{
		nb = &fc.fanet.neighbor[fc.fanet.neighbors_size];
		fc.fanet.neighbors_size += 1;

		nb->addr.manufacturer_id = new_data.addr.manufacturer_id;
		nb->addr.user_id = new_data.addr.user_id;

		nb->name[0] = 0;
		nb->flags = NB_VALID;
 	}

	nb->latitude = new_data.latitude;
	nb->longitude = new_data.longitude;
	nb->alititude = new_data.alititude;
	nb->heading = new_data.heading;
	nb->flags = (nb->flags & 0xF0) | (new_data.flags & 0x0F);
	nb->timestamp = HAL_GetTick() / 1000;

	bool FAI = config_get_select(&config.settings.units.earth_model) == EARTH_FAI;
	uint32_t dist = gnss_distance(nb->latitude, nb->longitude, fc.gnss.latitude, fc.gnss.longtitude, FAI, NULL);

	nb->dist = min(NB_TOO_FAR, dist);

	neighbors_update_magic();
}

void neighbors_update_name(fanet_addr_t addr, char * name)
{
	neighbor_t * nb = neighbors_find(addr);
	if (nb != NULL)
	{
		memcpy(&nb->name, name, NB_NAME_LEN);
		nb->name[NB_NAME_LEN - 1] = 0;
	}

	neighbors_update_magic();
}

void neighbors_step()
{

}
