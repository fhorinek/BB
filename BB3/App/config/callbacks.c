#include "config.h"

void dev_name_cb(cfg_entry_t * entry)
{
    if (strlen(config_get_text(entry)) == 0)
    {
        char dev_name[DEV_NAME_LEN];
        sprintf(dev_name, "Strato_%lX", DEVICE_ID);
        config_set_text(entry, dev_name);
    }
}

cfg_callback_pair_t config_callbacks[] =
{
    {&config.device_name, dev_name_cb}
};

static bool config_callbacks_enabled = true;

void config_process_cb(cfg_entry_t * entry)
{
    if (!config_callbacks_enabled)
        return;

    for (uint16_t i = 0; i < sizeof(config_callbacks) / sizeof(cfg_callback_pair_t); i++)
    {
        if (entry == config_callbacks[i].entry)
        {
            config_callbacks[i].cb(entry);
            return;
        }
    }
}

void config_trigger_callbacks()
{
    for (uint16_t i = 0; i < sizeof(config_callbacks) / sizeof(cfg_callback_pair_t); i++)
    {
        config_callbacks[i].cb(config_callbacks[i].entry);
    }
}

void config_disable_callbacks()
{
    config_callbacks_enabled = false;
}

void config_enable_callbacks()
{
    config_callbacks_enabled = false;
}
