#include "tile.h"

#include "../../fc/fc.h"
#include "../../fc/agl.h"

void tile_create(uint16_t * buffer, uint8_t w, int32_t lat, int32_t lon, int16_t angle, float zoom)
{
	uint32_t time_start = HAL_GetTick();

	float dlon, dlat;

	float rad = to_radians(angle);

	dlon = 2.1*GNSS_MUL/240.0;
	dlat = 2.1*GNSS_MUL/240.0;

	//dlon = GNSS_MUL/3601.0;
	//dlat = GNSS_MUL/3601.0;

	DBG("dlon %0.8f, dlat %0.8f", dlon / GNSS_MUL, dlat / GNSS_MUL);

	float flon, flat;

	flon = lon;
	flat = lat;

	int16_t vmin = 100;
	int16_t vmax = 1300;

	int16_t delta = vmax - vmin;

	for (uint8_t y = 0; y < w; y++)
	{
		for (uint8_t x = 0; x < w; x++)
		{
			uint16_t index = (y * w) + x;

			uint8_t r,g,b;

	 		r = g = b = 0;

			int16_t val = agl_get_alt(flat, flon, false);

			if (val == AGL_INVALID)
			{
				g = b = 0;
				r = 255;
			}
			else
			{
				if (val > vmax)
				{
					DBG("max is %i", val);
					val = vmax;
				}
				if (val < vmin)
				{
					DBG("min is %i", val);
					val = vmin;
				}

				val -= vmin;
				val = (val * 255) / delta;

				g = 255 - val;
				r = val;
			}

	 		//convert 24bit to 16bit (565)
	 		r >>= 3;
	 		g >>= 2;
	 		b >>= 3;

			buffer[index] = (r << 11) | (g << 5) | b;

			flon += dlon;
		}

		flon = lon;
		flat -= dlat;
	}

	DBG("time %lu", HAL_GetTick() - time_start);


}

