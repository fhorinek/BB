#include "pages.h"

#include "../../config/config.h"

uint8_t pages_get_count()
{
	uint8_t cnt = 0;

	for (uint8_t i = 0; i < PAGE_MAX_COUNT; i++)
	{
		if (config_get_text(&config.ui.page[i])[0] != 0)
			cnt++;
	}

	return cnt;
}

void pages_defragment()
{
	for (uint8_t i = 0; i < PAGE_MAX_COUNT; i++)
	{
		if (config_get_text(&config.ui.page[i])[0] == 0)
		{
			for (uint8_t j = i + 1; j < PAGE_MAX_COUNT; j++)
			{
				if (config_get_text(&config.ui.page[j])[0] != 0)
				{
					config_set_text(&config.ui.page[i], config_get_text(&config.ui.page[j]));
					config_set_text(&config.ui.page[j], "");
					break;
				}
			}
		}
	}
}

char * pages_get_name(uint8_t index)
{
	if (index < pages_get_count())
		return config_get_text(&config.ui.page[index]);

	return config_get_text(&config.ui.page[0]);
}
