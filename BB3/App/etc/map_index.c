/*
 * map_index.c
 *
 *  Created on: 23. 3. 2023
 *      Author: horinek
 */

#include "map_index.h"
#include "config/db.h"

void map_tiles_add(char * key, char * value)
{

    char flags[5] = "____";

    char path[PATH_LEN];
    snprintf(path, sizeof(path), PATH_TOPO_DIR "/%s.HGT", key);
    if (file_exists(path))
        flags[0] = 'A';

    snprintf(path, sizeof(path), PATH_MAP_DIR "/%s.MAP", key);
    bool have_map = file_exists(path);
    if (file_exists(path))
        flags[1] = 'M';

    snprintf(path, sizeof(path), PATH_MAP_DL_DIR "/%s.zip", key);
    if (file_exists(path))
        flags[1] = 'Z';

    INFO("ADDED %s %s", key, flags);
    db_insert(PATH_TILES_LISTS, key, flags);
}

void map_list_cb(char * key, char * value)
{
    char path[PATH_LEN];

    snprintf(path, sizeof(path), "%s/%s.list", PATH_MAP_LISTS, key);
    db_iterate(path, map_tiles_add);
    db_close_path(path);
}

void map_index_rebuild()
{
    INFO("Rebuilding index");
    db_drop(PATH_TILES_LISTS);
    db_iterate(PATH_MAP_SELECTED, map_list_cb);
    INFO("Rebuilding done");
}
