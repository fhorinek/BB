
static bool shrt_wifi_tgl_action()
{
    bool val = config_get_bool(&profile.wifi.enabled);

    INFO("shrt_wifi_tgl_action %u", val);

    config_set_bool(&profile.wifi.enabled, !val);

    if (!val)
        pages_popup("Wi-Fi on");
    else
        pages_popup("Wi-Fi off");

    return false;
}

static bool shrt_wifi_tgl_icon(char * icon, char * label)
{
    strcpy(icon, LV_SYMBOL_WIFI);
    strcpy(label, "Wi-Fi enable/disable");

    return true;
}

