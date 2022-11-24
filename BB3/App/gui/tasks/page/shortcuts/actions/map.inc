
static bool shrt_map_zoom_in_action()
{
    int16_t zoom = config_get_int(&profile.map.zoom_flight);
    int16_t new_zoom = zoom - 1;
    config_set_int(&profile.map.zoom_flight, new_zoom);
    zoom = config_get_int(&profile.map.zoom_flight);

    return new_zoom == zoom;
}

static bool shrt_map_zoom_in_icon(char * icon, char * label)
{
    int16_t zoom = config_get_int(&profile.map.zoom_flight);

    if (zoom > MAP_ZOOM_RANGE_FIRST)
        strcpy(icon, MD_PLUS_CIRCLE);
    else
        strcpy(icon, "");

    strcpy(label, _("Map zoom in"));

    return true;
}

static bool shrt_map_zoom_out_action()
{
    int16_t zoom = config_get_int(&profile.map.zoom_flight);
    int16_t new_zoom = zoom + 1;
    config_set_int(&profile.map.zoom_flight, new_zoom);
    zoom = config_get_int(&profile.map.zoom_flight);

    return new_zoom == zoom;
}

static bool shrt_map_zoom_out_icon(char * icon, char * label)
{
    int16_t zoom = config_get_int(&profile.map.zoom_flight);

    if (zoom <= MAP_ZOOM_RANGE_LAST)
        strcpy(icon, MD_MINUS_CIRCLE);
    else
        strcpy(icon, "");

    strcpy(label, _("Map zoom out"));

    return true;
}





