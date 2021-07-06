#include "pages.h"

#include "config/config.h"
#include "gui/widgets/widgets.h"

uint8_t pages_get_count()
{
	uint8_t cnt = 0;

	for (uint8_t i = 0; i < PAGE_MAX_COUNT; i++)
	{
		if (config_get_text(&profile.ui.page[i])[0] != 0)
			cnt++;
	}

	return cnt;
}

void pages_defragment()
{
	for (uint8_t i = 0; i < PAGE_MAX_COUNT; i++)
	{
		if (config_get_text(&profile.ui.page[i])[0] == 0)
		{
			for (uint8_t j = i + 1; j < PAGE_MAX_COUNT; j++)
			{
				if (config_get_text(&profile.ui.page[j])[0] != 0)
				{
					config_set_text(&profile.ui.page[i], config_get_text(&profile.ui.page[j]));
					config_set_text(&profile.ui.page[j], "");
					break;
				}
			}
		}
	}
}

char * pages_get_name(uint8_t index)
{
	if (index < pages_get_count())
		return config_get_text(&profile.ui.page[index]);

	return config_get_text(&profile.ui.page[0]);
}

bool page_rename(char * old_name, char * new_name)
{
	char path_old[PATH_LEN];
	char path_new[PATH_LEN];

	snprintf(path_new, sizeof(path_new), "%s/%s.pag", PATH_PAGES_DIR, new_name);
	snprintf(path_old, sizeof(path_old), "%s/%s.pag", PATH_PAGES_DIR, old_name);

	return f_rename(path_old, path_new) == FR_OK;
}

bool page_create(char * new_name)
{
	char path_new[PATH_LEN];

	snprintf(path_new, sizeof(path_new), "%s/%s.pag", PATH_PAGES_DIR, new_name);

	if (!file_exists(path_new))
	{
		page_layout_t page;
		page.base = NULL;
		page.number_of_widgets = 0;
		page.widget_slots = NULL;

		widgets_save_to_file(&page, new_name);

		return true;
	}
	else
	{
		return false;
	}
}

void page_delete(char * name)
{
	char path[64];

	snprintf(path, sizeof(path), "%s/%s.pag", PATH_PAGES_DIR, name);

	f_unlink(path);
}
