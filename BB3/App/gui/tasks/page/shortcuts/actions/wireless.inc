
static bool shrt_wifi_tgl_action()
{
    bool val = config_get_bool(&profile.wifi.enabled);

    config_set_bool(&profile.wifi.enabled, !val);

    if (!val)
        pages_popup(_("Wi-Fi on"));
    else
        pages_popup(_("Wi-Fi off"));

    return false;
}

static bool shrt_wifi_tgl_icon(char * icon, char * label)
{
    bool val = config_get_bool(&profile.wifi.enabled);

    if (!val)
        strcpy(icon, MD_WIFI);
    else
        strcpy(icon, MD_WIFI_OFF);

    strcpy(label, _("Wi-Fi on/off"));

    return true;
}


static bool shrt_bt_tgl_action()
{
    bool val = config_get_bool(&profile.bluetooth.enabled);

    config_set_bool(&profile.bluetooth.enabled, !val);

    if (!val)
        pages_popup(_("Bluetooth on"));
    else
        pages_popup(_("Bluetooth off"));

    return false;
}

static bool shrt_bt_tgl_icon(char * icon, char * label)
{
    bool val = config_get_bool(&profile.bluetooth.enabled);

    if (!val)
        strcpy(icon, MD_BLUETOOTH);
    else
        strcpy(icon, MD_BLUETOOTH_OFF);

    strcpy(label, _("Bluetooth on/off"));

    return true;
}

