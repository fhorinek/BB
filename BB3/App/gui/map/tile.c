/*
 * tile.c
 *
 *  Created on: 10. 11. 2020
 *      Author: horinek
 */

#define DEBUG_LEVEL DBG_DEBUG

#include "tile.h"
#include "linked_list.h"
#include "map_thread.h"

#include "fc/fc.h"
#include "fc/agl.h"
#include "etc/geo_calc.h"

#define MAP_BUFFER_SIZE (MAP_W * MAP_H * sizeof(lv_color_t))


// Some HGT files contain 1201 x 1201 points (3 arc/90m resolution)
#define HGT_DATA_WIDTH_3        1201ul

// Some HGT files contain 3601 x 3601 points (1 arc/30m resolution)
#define HGT_DATA_WIDTH_1        3601ul

// Some HGT files contain 3601 x 1801 points (1 arc/30m resolution)
#define HGT_DATA_WIDTH_1_HALF   1801ul

#define POS_FLAG_NOT_FOUND  0b00000001
#define POS_INVALID 0x00, -128, -32768

#define AGL_INVALID -32768



typedef struct
{
    uint8_t id;
    uint8_t magic;
    uint8_t grid_w;
    uint8_t grid_h;
    int32_t longitude;
    int32_t latitude;
} map_header_t;

typedef struct
{
    uint32_t index_addr;
    uint32_t feature_cnt;
} map_info_entry_t;

#define CACHE_START_WORD    0x55AA
#define CACHE_VERSION       2

#define CACHE_HAVE_AGL      0b10000000
#define CACHE_HAVE_MAP_MASK 0b01111111


typedef struct
{
    uint16_t start_word;
    uint16_t version;

    uint8_t src_files_magic[4]; //11 12 22 21

    uint16_t number_of_poi;
    uint8_t _pad[2];
} cache_header_t;


typedef struct
{
    uint8_t x;
    uint8_t y;
    uint8_t type;
    uint8_t name_len;

    uint32_t uid;
} cache_poi_t;

typedef struct
{
    int32_t lat;
    int32_t lon;
    uint32_t size;
} agl_header_t;

#define MAP_SHADE_MAG   150
//#define MAP_SHADE_MAG   0

#define BLUR_SIZE   3

void draw_polygon(lv_obj_t * canvas, lv_point_t * points, uint16_t number_of_points, lv_draw_line_dsc_t * draw_desc);

void write_buffer(char * path, void * buffer, uint32_t len)
{
    return;

    FIL f;
    UINT wb;

    f_open(&f, path, FA_WRITE | FA_CREATE_NEW);
    f_write(&f, buffer, len, &wb);
    f_close(&f);
}


//TODO: optimize to 1/4 size
int16_t * generate_blur_kernel(uint16_t size)
{
    FASSERT(size % 2 == 1);
    // initialising standard deviation to 1.0

    float * tmp_kernel = malloc(sizeof(float) * size * size);
    int16_t * kernel = ps_malloc(sizeof(int16_t) * size * size);

    uint16_t one_half = size / 2;

    float sigma = 1.0;
    float r, s = size * sigma * sigma;

    // sum is for normalization
    float sum = 0.0;

    // generate kernel
    uint16_t index = 0;
    for (int16_t y = -one_half; y <= one_half; y++)
    {
        for (int16_t x = -one_half; x <= one_half; x++)
        {
            r = sqrt(x * x + y * y);
            tmp_kernel[index] = (exp(-(r * r) / s)) / (M_PI * s);
            sum += tmp_kernel[index];
            index++;
        }
    }

    // normalising the Kernel
    for (int16_t i = 0; i < size * size; ++i)
    {
        tmp_kernel[i] /= sum;
        kernel[i] = tmp_kernel[i] * 1024;
        INFO("kernel[%u]: %d", i, kernel[i]);
    }

    write_buffer("kernel.data", kernel, sizeof(int16_t) * size * size);

    free(tmp_kernel);

    return kernel;
}

typedef struct
{
    int16_t h;
    int16_t s;
    int16_t v;
    int16_t steps;
} palete_hsv_point_t;

lv_color_t * generate_palette_hsv(palete_hsv_point_t * pts, uint8_t cnt, uint16_t * pal_len)
{
    *pal_len = 0;
    for (uint8_t i = 0; i < cnt-1; i++)
    {
        *pal_len += pts[i].steps;
    }

    lv_color_t * palette = malloc(sizeof(lv_color_t) * *pal_len);
    uint16_t index = 0;

    for (uint8_t i = 0; i < cnt-1; i++)
    {
        int16_t h1 = pts[i].h;
        int16_t s1 = pts[i].s;
        int16_t v1 = pts[i].v;
        int16_t h2 = pts[i + 1].h;
        int16_t s2 = pts[i + 1].s;
        int16_t v2 = pts[i + 1].v;

        uint16_t steps = pts[i].steps;

        for (uint16_t j = 0; j < steps; j++)
        {
            int16_t h = h1 + (j * (h2-h1)) / steps;
            h = (h + 360) % 360;
            int16_t s = s1 + (j * (s2-s1)) / steps;
            int16_t v = v1 + (j * (v2-v1)) / steps;
            palette[index++] = lv_color_hsv_to_rgb(h, s, v);
        }
    }

    return palette;
}

typedef struct
{
    int16_t r;
    int16_t g;
    int16_t b;
    int16_t steps;
} palete_rgb_point_t;

lv_color_t * generate_palette_rgb(palete_rgb_point_t * pts, uint8_t cnt, uint16_t * pal_len)
{
    *pal_len = 0;
    for (uint8_t i = 0; i < cnt-1; i++)
    {
        *pal_len += pts[i].steps;
    }

    lv_color_t * palette = malloc(sizeof(lv_color_t) * *pal_len);
    uint16_t index = 0;

    for (uint8_t i = 0; i < cnt-1; i++)
    {
        int16_t r1 = pts[i].r;
        int16_t g1 = pts[i].g;
        int16_t b1 = pts[i].b;
        int16_t r2 = pts[i + 1].r;
        int16_t g2 = pts[i + 1].g;
        int16_t b2 = pts[i + 1].b;

        uint16_t steps = pts[i].steps;

        for (uint16_t j = 0; j < steps; j++)
        {
            int16_t r = r1 + (j * (r2-r1)) / steps;
            int16_t g = g1 + (j * (g2-g1)) / steps;
            int16_t b = b1 + (j * (b2-b1)) / steps;
            palette[index++] = lv_color_make(r, g, b);
        }
    }

    return palette;
}


#define MAP_W_BLUR_SHADE        (MAP_W + BLUR_SIZE + 2)
#define MAP_H_BLUR_SHADE        (MAP_H + BLUR_SIZE + 2)
#define MAP_BLUR_SHADE_OFFSET   (BLUR_SIZE / 2 + 1)
#define MAP_BLUR_SHADE          (BLUR_SIZE / 2)

uint8_t load_agl_data(int32_t lon1, int32_t lat1, int32_t lon2, int32_t lat2, int32_t step_x, int32_t step_y, uint8_t * input, int16_t * output)
{
    if (input == NULL)
        return 0;

    agl_header_t * ah = ((agl_header_t *)input);
    int16_t * agl_data = (int16_t *)(input + sizeof(agl_header_t));

    if (ah->size != HGT_DATA_WIDTH_3 * HGT_DATA_WIDTH_3 * 2)
    {
        return 0;
    }

    uint16_t num_points_x = HGT_DATA_WIDTH_3;
    uint16_t num_points_y = HGT_DATA_WIDTH_3;

    int16_t y_start = max(-MAP_BLUR_SHADE_OFFSET, (lat1 - (ah->lat + GNSS_MUL)) / step_y);
    int16_t y_end = min(MAP_H + MAP_BLUR_SHADE_OFFSET, (lat1 - ah->lat) / step_y);
    int16_t x_start = max(-MAP_BLUR_SHADE_OFFSET, (ah->lon - lon1) / step_x);
    int16_t x_end = min(MAP_W + MAP_BLUR_SHADE_OFFSET, (ah->lon + GNSS_MUL - lon1) / step_x);


    uint32_t coord_div_x = GNSS_MUL / (num_points_x - 2);
    uint32_t coord_div_y = GNSS_MUL / (num_points_y - 2);

    for (int16_t y = y_start; y <= y_end; y++)
    {
        int32_t lat = lat1 - y * step_y;
        if (lat < ah->lat)
            lat = ah->lat;
        if (lat > ah->lat + GNSS_MUL - num_points_y)
            lat = ah->lat + GNSS_MUL - num_points_y;

        for (int16_t x = x_start; x <= x_end; x++)
        {
            int32_t lon = (x * step_x) + lon1;
            if (lon < ah->lon)
                lon = ah->lon;
            if (lon > ah->lon + GNSS_MUL - num_points_x)
                lon = ah->lon + GNSS_MUL - num_points_x;

            int32_t lat_mod = lat % GNSS_MUL;
            int32_t lon_mod = lon % GNSS_MUL;
            if (lat_mod < 0) lat_mod += GNSS_MUL;
            if (lon_mod < 0) lon_mod += GNSS_MUL;

            int32_t index_y = lat_mod / coord_div_y;
            int32_t index_x = lon_mod / coord_div_x;

            uint32_t pos = index_x + num_points_x * (num_points_y - index_y - 2);
            ASSERT(pos < num_points_x * num_points_y)

            int16_t alt11 = SWAP_UINT16(agl_data[pos]);
            int16_t alt21 = SWAP_UINT16(agl_data[pos + 1]);

            pos -= num_points_x;
            int16_t alt12 = SWAP_UINT16(agl_data[pos]);
            int16_t alt22 = SWAP_UINT16(agl_data[pos + 1]);

            float lat_dr = (lat_mod % coord_div_y) / (float)(coord_div_y);
            float lon_dr = (lon_mod % coord_div_x) / (float)(coord_div_x);

            //compute height by using bilinear interpolation
            float alt1 = alt11 + (float)(alt12 - alt11) * lat_dr;
            float alt2 = alt21 + (float)(alt22 - alt21) * lat_dr;

            int16_t alt = alt1 + (float)(alt2 - alt1) * lon_dr;
            uint32_t index = x + MAP_BLUR_SHADE_OFFSET + MAP_W_BLUR_SHADE * (y + MAP_BLUR_SHADE_OFFSET);
            output[index] = (int16_t)alt;
        }
    }

    write_buffer("step1.data", output, sizeof(int16_t) * MAP_W_BLUR_SHADE * MAP_H_BLUR_SHADE);
    return CACHE_HAVE_AGL;
}

void draw_topo(int32_t lon1, int32_t lat1, int32_t lon2, int32_t lat2, int32_t step_x, int32_t step_y, uint8_t * magic)
{
    //create static dest buffer for tile in psram
    static int16_t * topo_alt = NULL;
    static int16_t * topo_blur = NULL;
    static int16_t * blur_kernel = NULL;
    static lv_color_t * palette = NULL;
    static uint16_t palette_len;

    lv_canvas_ext_t * ext = lv_obj_get_ext_attr(gui.map.canvas);
    uint16_t * image_buff = (uint16_t *)ext->dsc.data;

    if (topo_alt == NULL)
    {
        //INIT MEMORY
        topo_alt = (int16_t *)ps_malloc(sizeof(int16_t) * MAP_W_BLUR_SHADE * MAP_H_BLUR_SHADE);
        topo_blur = (int16_t *)ps_malloc(sizeof(int16_t) * MAP_W_BLUR_SHADE * MAP_H_BLUR_SHADE);
        blur_kernel = generate_blur_kernel(BLUR_SIZE);

        palete_rgb_point_t pts[] = {
                {92, 145, 70, 50},
                {92, 145, 70, 100},
                {190, 190, 65, 100},
                {160, 100, 70, 100},
                {130, 90, 60, 100},
                {185, 185, 185, 300},
                {140, 140, 165, 400},
        };


        palette = generate_palette_rgb(pts, sizeof(pts) / sizeof(palete_hsv_point_t), &palette_len);

        write_buffer("palette.data", palette, sizeof(lv_color_t) * palette_len);
    }


    DBG("Loading agl data");
    uint32_t timestamp = HAL_GetTick();

    //LOAD DEFAULT HEIGHT VALUES
    for (uint32_t i = 0; i < MAP_W_BLUR_SHADE * MAP_H_BLUR_SHADE; i++)
    {
        topo_alt[i] = INT16_MIN;
    }
    memset(topo_blur, 0, sizeof(int16_t) * MAP_W_BLUR_SHADE * MAP_H_BLUR_SHADE);


    //Get tile names
    hagl_pos_t tile_pos[4];
    tile_pos[0] = agl_get_fpos(lon1, lat1);
    tile_pos[1] = agl_get_fpos(lon1, lat2);
    tile_pos[2] = agl_get_fpos(lon2, lat2);
    tile_pos[3] = agl_get_fpos(lon2, lat1);

    //remove duplicates
    for(uint8_t i = 0; i < 4; i++)
    {
        for (uint8_t j = i + 1; j < 4; j++)
        {
            if (agl_pos_cmp(&tile_pos[i], &tile_pos[j]))
                tile_pos[j].flags |= POS_FLAG_DUPLICATE;
        }
    }

    //buffer for map data
    static uint8_t * agl_cache = NULL;
    //name of the file in buffer
    static hagl_pos_t agl_cache_pos = {POS_INVALID};

    //first use cached file
    if (agl_cache != NULL)
    {
        for(uint8_t i = 0; i < 4; i++)
        {
            if (agl_pos_cmp(&tile_pos[i], &agl_cache_pos))
            {
                //tile loaded!
                load_agl_data(lon1, lat1, lon2, lat2, step_x, step_y, agl_cache, topo_alt);
                tile_pos[i].flags |= POS_FLAG_DONE;
                magic[i] |= CACHE_HAVE_AGL;
            }
        }
    }

    //load the rest
    for(uint8_t i = 0; i < 4; i++)
    {
        //if not duplicate or done
        if ((tile_pos[i].flags & (POS_FLAG_DUPLICATE | POS_FLAG_DONE)) == 0)
        {
            FIL agl_data;

            char path[PATH_LEN];
            char name[12];
            agl_get_filename(name, tile_pos[i]);
            snprintf(path, sizeof(path), "%s/%s.HGT", PATH_TOPO_DIR, name);

            if (f_open(&agl_data, path, FA_READ) != FR_OK)
            {
                ERR("agl file %s not found", name);
                db_insert(PATH_TOPO_INDEX, name, "W"); //set want flag
                tile_pos[i].flags |= POS_FLAG_NOT_FOUND;
                continue;
            }

            if (agl_cache != NULL)
            {
                ps_free(agl_cache);
                agl_cache = NULL;
            }

            UINT br;
            agl_cache = ps_malloc(f_size(&agl_data) + sizeof(agl_header_t));

            ((agl_header_t *)agl_cache)->lat = (tile_pos[i].lat * GNSS_MUL);
            ((agl_header_t *)agl_cache)->lon = (tile_pos[i].lon * GNSS_MUL);
            ((agl_header_t *)agl_cache)->size = f_size(&agl_data);

            INFO("Reading %s (%u)", name, f_size(&agl_data));
            FRESULT ret = f_read(&agl_data, agl_cache + sizeof(agl_header_t), f_size(&agl_data), &br);
            INFO("read %u", br);
            if (br != f_size(&agl_data) || ret != FR_OK)
            {
                WARN("Not all data read ret = %u", ret);
            }

            f_close(&agl_data);

            //mark pos to cache
            memcpy(&agl_cache_pos, &tile_pos[i], sizeof(hagl_pos_t));
            load_agl_data(lon1, lat1, lon2, lat2, step_x, step_y, agl_cache, topo_alt);

            tile_pos[i].flags |= POS_FLAG_DONE;
            magic[i] |= CACHE_HAVE_AGL;
        }
    }

    write_buffer("step1.data", topo_alt, sizeof(int16_t) * MAP_W_BLUR_SHADE * MAP_H_BLUR_SHADE);

    //no gnss data
    if (*((uint32_t *)magic) == 0)
    {
        for (uint16_t y = 0; y < MAP_H; y++)
        {
            for (uint16_t x = 0; x < MAP_W; x++)
            {
                image_buff[x + MAP_W * y] = LV_COLOR_GRAY.full;
            }
        }
        return;
    }

    DBG("Loading data done (%u)", HAL_GetTick() - timestamp);
    timestamp = HAL_GetTick();

    bool blur_enable = config_get_bool(&profile.map.blur);

    if (blur_enable)
    {
        //BLUR HEIGHT MAP
        for (int16_t y = -1; y < MAP_H + 1; y++)
        {
            for (int16_t x = -1; x < MAP_W + 1; x++)
            {
                int32_t sum = 0;
                for (uint16_t i = 0; i < BLUR_SIZE * BLUR_SIZE; i++)
                {
                    int16_t ox = i % BLUR_SIZE - (BLUR_SIZE / 2);
                    int16_t oy = i / BLUR_SIZE - (BLUR_SIZE / 2);

                    int32_t bindex = (x + ox  + MAP_BLUR_SHADE_OFFSET) + MAP_W_BLUR_SHADE * (y + oy  + MAP_BLUR_SHADE_OFFSET);
                    sum += blur_kernel[i] * topo_alt[bindex];
                }

                uint32_t index = (x + 1) + MAP_W_BLUR_SHADE * (y + 1);
                topo_blur[index] = sum / 1024;
            }
        }

        DBG("Bluring (%u)", HAL_GetTick() - timestamp);
    }

    write_buffer("step2.data", topo_blur, sizeof(int16_t) * MAP_W_BLUR_SHADE * MAP_H_BLUR_SHADE);

    int16_t * topo_src = (blur_enable) ? topo_blur : topo_alt;

    //CALC SHADE
    //ASSIGN COLORS
    int16_t val_min = 0000;
    int16_t val_max = 4000;
    switch (config_get_select(&profile.map.alt_range))
    {
        case(MAP_ALT_RANGE_FLAT):
            val_max = 2000;
        break;
        case(MAP_ALT_RANGE_NORMAL):
            val_max = 4000;
        break;
        case(MAP_ALT_RANGE_ALPS):
            val_max = 6000;
        break;
    }

    int16_t delta = val_max - val_min;
    if (delta == 0)
        delta = 1;

    int16_t step_x_m, step_y_m;
    geo_get_topo_steps((lat1 + lat2) / 2, step_x, step_y, &step_x_m, &step_y_m);
    int16_t step_d_m = sqrt(pow(step_x_m, 2) + pow(step_y_m, 2));

    timestamp = HAL_GetTick();
    for (uint16_t y = 0; y < MAP_H; y++)
    {
        for (uint16_t x = 0; x < MAP_W; x++)
        {
            uint32_t index;

            index = x + 1 - 1 + MAP_W_BLUR_SHADE * (y + 1 - 1);
            int16_t val_top_left = topo_src[index];
            int16_t val_top = topo_src[index + 1];
            int16_t val_top_right = topo_src[index + 2];

            index += MAP_W_BLUR_SHADE;
            int16_t val_left = topo_src[index];
            int16_t val_center = topo_src[index + 1];
            int16_t val_right = topo_src[index + 2];

            //not blurred
            int16_t val = topo_alt[index + 1];

            index += MAP_W_BLUR_SHADE;
            int16_t val_bottom_left = topo_src[index];
            int16_t val_bottom = topo_src[index + 1];
            int16_t val_bottom_right = topo_src[index + 2];

            //get near
            int32_t delta_hor = ((val_center - val_left) + (val_right - val_center)) / 2;
            int32_t delta_ver = ((val_center - val_top) + (val_bottom - val_center)) / 2;
            int32_t delta_dia1 = ((val_center - val_top_left) + (val_bottom_right - val_center)) / 2;
            int32_t delta_dia2 = ((val_center - val_bottom_left) + (val_top_right- val_center)) / 2;

            int16_t ilum = 0;
            ilum += (delta_hor * MAP_SHADE_MAG) / (step_x_m);
            ilum += (delta_ver * MAP_SHADE_MAG) / (step_y_m);
            ilum += (delta_dia1 * MAP_SHADE_MAG) / (step_d_m);
            ilum += (delta_dia2 * MAP_SHADE_MAG) / (step_d_m);

            //clamp
            ilum = min(ilum, LV_OPA_COVER);
            ilum = max(ilum, -LV_OPA_COVER);

            index = x + MAP_W * y;

            //get color
            lv_color_t color;
            if (val == INT16_MIN)
            {
                color = lv_color_make(60, 100, 130);
            }
            else
            {
                int16_t ci = min(palette_len - 1, ((val - val_min) * palette_len) / delta);
                ci = max(0, ci);
                color = palette[ci];
            }

            //apply shade
            if (ilum > 0)
                color = lv_color_lighten(color, ilum);
            else
                color = lv_color_darken(color, -ilum);

            index = x + MAP_W * y;
            image_buff[index] = color.full;
        }
    }

    write_buffer("step3.data", image_buff, sizeof(int16_t) * MAP_W * MAP_H);

    DBG("Finalising / Topo done (%u)", HAL_GetTick() - timestamp);
    timestamp = HAL_GetTick();
}



uint8_t tile_find_inside(int32_t lon, int32_t lat, uint16_t zoom)
{
    for (uint8_t i = 0; i < 9; i++)
    {
        if (!gui.map.chunks[i].ready)
        {
            continue;
        }

        if (gui.map.chunks[i].zoom != zoom)
        {
            continue;
        }

        int16_t x, y;
        tile_geo_to_pix(i, lon, lat, &x, &y);

        if (x >= 0 && x < MAP_W && y >= 0 && y < MAP_H)
            return i;
    }

    return 0xFF;
}

void tile_geo_to_pix(uint8_t index, int32_t g_lon, int32_t g_lat, int16_t * x, int16_t * y)
{
    int32_t lon = gui.map.chunks[index].center_lon;
    int32_t lat = gui.map.chunks[index].center_lat;
    uint16_t zoom = gui.map.chunks[index].zoom;

    geo_to_pix(lon, lat, zoom, g_lon, g_lat, x, y);
}

void tile_align_to_cache_grid(int32_t lon, int32_t lat, uint16_t zoom, int32_t * c_lon, int32_t * c_lat)
{
    int32_t step_x;
    int32_t step_y;
    geo_get_steps(lat, zoom, &step_x, &step_y);

    //get bbox
    uint32_t map_w = (MAP_W * step_x);
    uint32_t map_h = (MAP_H * step_y);

    uint32_t number_of_cols = 1 + GNSS_MUL / map_w;
    uint32_t number_of_rows = 1 + GNSS_MUL / map_h;

    int32_t col_div = GNSS_MUL / number_of_cols;
    int32_t row_div = GNSS_MUL / number_of_rows;

    if (lat > 0)
        *c_lat = (lat / GNSS_MUL) * GNSS_MUL + ((lat % GNSS_MUL) / row_div) * row_div + row_div / 2;
    else
        *c_lat = (lat / GNSS_MUL) * GNSS_MUL + ((lat % GNSS_MUL) / row_div) * row_div - row_div / 2;

    if (lon > 0)
        *c_lon = (lon / GNSS_MUL) * GNSS_MUL + ((lon % GNSS_MUL) / col_div) * col_div + col_div / 2;
    else
        *c_lon = (lon / GNSS_MUL) * GNSS_MUL + ((lon % GNSS_MUL) / col_div) * col_div - col_div / 2;
}

void tile_get_cache(int32_t lon, int32_t lat, uint16_t zoom, int32_t * c_lon, int32_t * c_lat, char * path)
{
    tile_align_to_cache_grid(lon, lat, zoom, c_lon, c_lat);

    sprintf(path, PATH_MAP_CACHE_DIR "/%u/%08lX%08lX", zoom, *c_lon, *c_lat);
}

void tile_get_filename(char * fn, int32_t lon, int32_t lat)
{
    hagl_pos_t tmp = agl_get_fpos(lon, lat);
    agl_get_filename(fn, tmp);
}

static uint8_t * load_map_file(int32_t lon, int32_t lat, uint8_t index)
{
    //buffer for map data
    static uint8_t * map_cache = NULL;
    //name of the file in buffer
    static char map_cache_name[16];

    //names used to generate tile
    static char name[4][16];

    tile_get_filename(name[index], lon, lat);

    for (uint8_t i = index; i > 0; i--)
    {
        //was this file already processed?
        if (strcmp(name[i-1], name[index]) == 0)
            return NULL;
    }

    bool loaded = false;

    if (map_cache != NULL)
    {
        if (strcmp(map_cache_name, name[index]) == 0)
        {
            loaded = true;
        }
    }

    if (!loaded)
    {
        FIL map_data;

        char path[PATH_LEN];
        snprintf(path, sizeof(path), "%s/%s.MAP", PATH_MAP_DIR, name[index]);
        if (f_open(&map_data, path, FA_READ) != FR_OK)
        {
            ERR("map file %s not found", name[index]);
            db_insert(PATH_MAP_INDEX, name[index], "W"); //set want flag
            return NULL;
        }

        if (map_cache != NULL)
        {
            ps_free(map_cache);
            map_cache = NULL;
        }

        UINT br;
        map_cache = ps_malloc(f_size(&map_data));

        f_read(&map_data, map_cache, f_size(&map_data), &br);
        f_close(&map_data);

        //mark name to cache
        strcpy(map_cache_name, name[index]);
    }

    //check if this file was already used on this tile
    return map_cache;
}

void tile_unload_pois(uint8_t index)
{
    for (uint8_t i = 0; i < NUMBER_OF_POI; i++)
    {
        if (gui.map.poi[i].chunk == index)
        {
            free(gui.map.poi[i].name);
            gui.map.poi[i].name = NULL;
            gui.map.poi[i].chunk = 0xFF;
            gui.map.poi_size--;
        }

    }
}

bool tile_find_poi(uint32_t uid)
{
    for (uint8_t i = 0; i < NUMBER_OF_POI; i++)
    {
        if (gui.map.poi[i].chunk != 0xFF)
        {
            if (gui.map.poi[i].uid == uid)
                return true;
        }
    }
    return false;
}

bool tile_poi_add(map_poi_t *poi, char *name, uint16_t name_len)
{
    if (gui.map.poi_size >= NUMBER_OF_POI)
        return false;

    bool done = false;

    //point already loaded?
    if (!tile_find_poi(poi->uid))
    {
        for (uint8_t i = 0; i < NUMBER_OF_POI && !done; i++)
        {
            //find free poi slot
            gui_lock_acquire();
            if (gui.map.poi[i].chunk == 0xFF)
            {
                gui.map.poi_size++;
                memcpy(&gui.map.poi[i], poi, sizeof(map_poi_t));

                gui.map.poi[i].name = malloc(name_len + 1);
                strncpy(gui.map.poi[i].name, name, name_len);
                gui.map.poi[i].name[name_len] = 0;

                done = true;
            }
            gui_lock_release();
        }

        return true;
    }
    return false;
}


uint8_t draw_map(int32_t lon1, int32_t lat1, int32_t lon2, int32_t lat2, int32_t step_x, int32_t step_y, uint16_t zoom, uint8_t * map_cache, uint8_t chunk_index)
{
    if (map_cache == NULL)
        return 0;

    static uint8_t poi_magic = 0xFF;
    poi_magic = (poi_magic + 1) % 0xFF;

    map_header_t * mh = (map_header_t *)map_cache;

//  DBG("file grid is %u x %u", mh->grid_w, mh->grid_h);

    if ((int64_t)lon1 - (int64_t)mh->longitude < - 300ll * GNSS_MUL)
    {
        lon1 += 360ll * GNSS_MUL;
        lon2 += 360ll * GNSS_MUL;
    }
    if ((int64_t)lon1 - (int64_t)mh->longitude > 300ll * GNSS_MUL)
    {
        lon1 -= 360ll * GNSS_MUL;
        lon2 -= 360ll * GNSS_MUL;
    }

    int32_t flon = (mh->longitude / GNSS_MUL) * GNSS_MUL;
    int32_t flat = (mh->latitude / GNSS_MUL) * GNSS_MUL;

    uint32_t grid_start_addr = sizeof(map_header_t);

    //search the grid and locate the features that are visible
    int32_t gstep_x = GNSS_MUL / mh->grid_w;
    int32_t gstep_y = GNSS_MUL / mh->grid_h;

//  INFO("rectangle name=disp bbox=%f,%f,%f,%f", lon1 / (float)GNSS_MUL, lat1 / (float)GNSS_MUL, lon2 / (float)GNSS_MUL, lat2 / (float)GNSS_MUL);

    ll_item_t * start = NULL;
    ll_item_t * end = NULL;

    for (uint8_t y = 0; y < mh->grid_h; y++)
    {
        int32_t glat1 = flat + gstep_y * y;
        int32_t glat2 = glat1 + gstep_y;

        if ((glat1 <= lat1 && lat1 <= glat2)
                || (glat1 <= lat2 && lat2 <= glat2)
                || (lat1 >= glat1 && glat2 >= lat2))
        {
            for (uint8_t x = 0; x < mh->grid_w; x++)
            {
                int32_t glon1 = flon + gstep_x * x;
                int32_t glon2 = glon1 + gstep_x;

                if ((glon1 <= lon1 && lon1 <= glon2)
                        || (glon1 <= lon2 && lon2 <= glon2)
                        || (lon1 <= glon1 && glon2 <= lon2))
                {
//                  INFO("rectangle name=x%uy%u bbox=%f,%f,%f,%f", x, y,
//                          glon1 / (float)GNSS_MUL, glat1 / (float)GNSS_MUL,
//                          glon2 / (float)GNSS_MUL, glat2 / (float)GNSS_MUL);

                    //load features
                    uint32_t grid_addr = grid_start_addr + (y * mh->grid_h + x) * sizeof(map_info_entry_t);

                    map_info_entry_t * in = (map_info_entry_t * )(map_cache + grid_addr);

                    for (uint16_t i = 0; i < in->feature_cnt; i++)
                    {
                        uint32_t feature_addr = *((uint32_t *)(map_cache + in->index_addr + 4 * i));

                        list_add_sorted_unique(feature_addr, &start, &end);
                    }
                }
            }
        }
    }

    lv_draw_rect_dsc_t poly_draw;
    lv_draw_rect_dsc_init(&poly_draw);

    lv_draw_line_dsc_t line_draw;
    lv_draw_line_dsc_init(&line_draw);
    lv_draw_line_dsc_t warn_line_draw;
    lv_draw_line_dsc_init(&warn_line_draw);

    warn_line_draw.width = 3;
    warn_line_draw.color = LV_COLOR_RED;
    warn_line_draw.round_end = 1;
    warn_line_draw.round_start = 1;

    lv_draw_label_dsc_t text_draw;
    lv_draw_label_dsc_init(&text_draw);
    text_draw.font = &lv_font_montserrat_12;
    text_draw.color = LV_COLOR_BLACK;

//  lv_draw_rect_dsc_t rect;
//  lv_draw_rect_dsc_init(&rect);
//  rect.border_width = 1;
//  rect.bg_opa = LV_OPA_TRANSP;
//
//  lv_canvas_draw_rect(gui.map.canvas, 0, 0, MAP_W, MAP_H, &rect);

    ll_item_t * actual = start;

    //draw features
    while(actual != NULL)
    {
        uint8_t type = *((uint8_t *)(map_cache + actual->feature_addr));
//      DBG("feature %u type %u", index++, type);

//        if (type == 0 || (type <= 13 && type >= 10)) //place
        if (type <= 13 && type >= 10) //place
        {
            int32_t plon = *((int32_t *)(map_cache + actual->feature_addr + 4));
            int32_t plat = *((int32_t *)(map_cache + actual->feature_addr + 8));

            int32_t clon = lon1 + (lon2 - lon1) / 2;
            int32_t clat = lat1 + (lat2 - lat1) / 2;

            int16_t x, y;
            geo_to_pix_w_h(clon, clat, zoom, plon, plat, &x, &y, MAP_W, MAP_H);

            if (!(x < 0 || x >= MAP_W || y < 0 || y >= MAP_H))
            {
                map_poi_t poi;

                poi.chunk = chunk_index;
                poi.magic = poi_magic;
                poi.type = type;
                poi.uid = actual->feature_addr;

                poi.x = x;
                poi.y = y;

                uint8_t name_len = *((uint8_t *)(map_cache + actual->feature_addr + 1));
                tile_poi_add(&poi, (char *)(map_cache + actual->feature_addr + 12), name_len);
            }
        }

        bool draw_warning = false;

        if (type / 100 == 1) //lines
        {
            bool skip = false;

            line_draw.opa = LV_OPA_COVER;

            switch (type)
            {
            //border
            case(100):
                line_draw.width = 4;
                line_draw.color = LV_COLOR_NAVY;
                break;
            //river
            case(110):
                line_draw.width = 2;
                line_draw.color = lv_color_make(60, 100, 130);
                break;
            //road
            case(120):
                line_draw.width = 2;
                line_draw.color = lv_color_make(225, 170, 0);
                break;
            case(121):
                line_draw.width = 2;
                line_draw.color = LV_COLOR_YELLOW;
                break;
            case(122):
                line_draw.width = 1;
                line_draw.color = LV_COLOR_WHITE;
                break;
            case(123):
                if (zoom > 30)
                    skip = true;

                line_draw.width = 1;
                line_draw.color = LV_COLOR_WHITE;
                break;
            //rail
            case(130):
                line_draw.width = 1;
                line_draw.color = LV_COLOR_BLACK;
                break;
            //power or airway
            default:
                line_draw.width = 1;
                line_draw.color = LV_COLOR_BLACK;
                draw_warning = true;
                break;

            }

            if (!skip)
            {
                uint16_t number_of_points = *((uint16_t *)(map_cache + actual->feature_addr + 2));

                lv_point_t * points = (lv_point_t *) malloc(sizeof(lv_point_t) * number_of_points);
                for (uint16_t j = 0; j < number_of_points; j++)
                {
                    int32_t plon, plat;

                    plon = *((int32_t *)(map_cache + actual->feature_addr + 4 + 0 + 8 * j));
                    plat = *((int32_t *)(map_cache + actual->feature_addr + 4 + 4 + 8 * j));

                    int64_t px = (int64_t)(plon - lon1) / step_x;
                    int64_t py = (int64_t)(lat1 - plat) / step_y;

                    if (px > INT16_MAX) px = INT16_MAX;
                    if (px < INT16_MIN) px = INT16_MIN;
                    if (py > INT16_MAX) py = INT16_MAX;
                    if (py < INT16_MIN) py = INT16_MIN;

                    points[j].x = px;
                    points[j].y = py;
                }

                gui_lock_acquire();
                if (draw_warning)
                    lv_canvas_draw_line(gui.map.canvas, points, number_of_points, &warn_line_draw);

                lv_canvas_draw_line(gui.map.canvas, points, number_of_points, &line_draw);
                gui_lock_release();

                free(points);
            }
        }

        if (type == 200 || type == 201) //water or resident
        {

            line_draw.width = 1;
            if (type == 200)
            {
                //water
                line_draw.color = lv_color_make(60, 100, 130);
                line_draw.opa = LV_OPA_COVER;
            }
            else
            {
                //resident
                line_draw.color = LV_COLOR_WHITE;
                line_draw.opa = LV_OPA_50;
            }

            uint16_t number_of_points = *((uint16_t *)(map_cache + actual->feature_addr + 2));

//          INFO("points = []");

            lv_point_t * points = (lv_point_t *) malloc(sizeof(lv_point_t) * number_of_points);
            for (uint16_t j = 0; j < number_of_points; j++)
            {
                int32_t plon, plat;

                plon = *((int32_t *)(map_cache + actual->feature_addr + 4 + 0 + 8 * j));
                plat = *((int32_t *)(map_cache + actual->feature_addr + 4 + 4 + 8 * j));

                int16_t px = ((int64_t)plon - (int64_t)lon1) / step_x;
                int16_t py = ((int64_t)lat1 - (int64_t)plat) / step_y;

                //multipolygon separator
                if (plat == 0x7FFFFFFF && plon == 0x7FFFFFFF)
                {
                    px = 0x7FFF;
                    py = 0x7FFF;
                }

                points[j].x = px;
                points[j].y = py;

//              INFO("points.append([%d, %d])", px, py);
            }
//          INFO("draw_poly(points)\n");

            draw_polygon(gui.map.canvas, points, number_of_points, &line_draw);

            free(points);
        }

        actual = actual->next;
    }
    list_free(start, end);

    return mh->magic & CACHE_HAVE_MAP_MASK;
}

void tile_create(int32_t lon1, int32_t lat1, int32_t lon2, int32_t lat2, int32_t step_x, int32_t step_y, uint16_t zoom,  uint8_t * magic, uint8_t chunk_index)
{
    //draw map map
    //TODO: draw loaded first like hagl
    magic[0] |= draw_map(lon1, lat1, lon2, lat2, step_x, step_y, zoom, load_map_file(lon1, lat1, 0), chunk_index);
    magic[1] |= draw_map(lon1, lat1, lon2, lat2, step_x, step_y, zoom, load_map_file(lon1, lat2, 1), chunk_index);
    magic[2] |= draw_map(lon1, lat1, lon2, lat2, step_x, step_y, zoom, load_map_file(lon2, lat2, 2), chunk_index);
    magic[3] |= draw_map(lon1, lat1, lon2, lat2, step_x, step_y, zoom, load_map_file(lon2, lat1, 3), chunk_index);
}

//get map source files on device
bool tile_validate_sources(int32_t lon1, int32_t lat1, int32_t lon2, int32_t lat2, uint8_t * magic)
{
    //Get tile names
    hagl_pos_t tile_pos[4];
    tile_pos[0] = agl_get_fpos(lon1, lat1);
    tile_pos[1] = agl_get_fpos(lon1, lat2);
    tile_pos[2] = agl_get_fpos(lon2, lat2);
    tile_pos[3] = agl_get_fpos(lon2, lat1);

    //remove duplicates
    for(uint8_t i = 0; i < 4; i++)
    {
        for (uint8_t j = i + 1; j < 4; j++)
        {
            if (agl_pos_cmp(&tile_pos[i], &tile_pos[j]))
                tile_pos[j].flags |= POS_FLAG_DUPLICATE;
        }
    }

    for (uint8_t i = 0; i < 4; i++)
    {
        if (tile_pos[i].flags & POS_FLAG_DUPLICATE)
            continue;

        char name[16];
        char path[PATH_LEN];

        agl_get_filename(name, tile_pos[i]);

        uint8_t tmp_magic = 0;

        snprintf(path, sizeof(path), "%s/%s.hgt", PATH_TOPO_DIR, name);
        if (file_exists(path))
            tmp_magic = CACHE_HAVE_AGL;

        snprintf(path, sizeof(path), "%s/%s.map", PATH_MAP_DIR, name);
        FIL f;
        if (f_open(&f, path, FA_READ) == FR_OK)
        {
            map_header_t mh;
            UINT rb;
            f_read(&f, &mh, sizeof(mh), &rb);
            tmp_magic |= mh.magic & CACHE_HAVE_MAP_MASK;
            f_close(&f);
        }

        INFO("magic[%u] = %02X %02X", i, magic[i], tmp_magic);

        if (tmp_magic != magic[i])
            return false;
    }

    return true;
}


bool tile_load_cache(uint8_t index, int32_t lon, int32_t lat, uint16_t zoom)
{
    gui.map.chunks[index].ready = false;
    gui.map.magic = (gui.map.magic + 1) % 0xFF;

    char tile_path[PATH_LEN];
    int32_t c_lon, c_lat;

    bool pass = true;
    tile_get_cache(lon, lat, zoom, &c_lon, &c_lat, tile_path);

    int32_t step_x;
    int32_t step_y;
    geo_get_steps(c_lat, zoom, &step_x, &step_y);

    //get bbox
    uint32_t map_w = MAP_W * step_x;
    uint32_t map_h = (MAP_H * step_y);
    int32_t lon1 = c_lon - map_w / 2;
    int32_t lon2 = c_lon + map_w / 2;
    int32_t lat1 = c_lat + map_h / 2;
    int32_t lat2 = c_lat - map_h / 2;

    if (file_exists(tile_path))
    {
        DBG("Trying to load from cache");

        //load from cache
        FIL f;
        UINT br;
        cache_header_t ch;
        FRESULT res;


        res = f_open(&f, tile_path, FA_READ);
        if (res == FR_OK)
        {
            res = f_read(&f, &ch, sizeof(cache_header_t), &br);
            if (res == FR_OK && br == sizeof(cache_header_t))
            {
                if (ch.start_word != CACHE_START_WORD || ch.version != CACHE_VERSION)
                {
                    WARN("Cache header invalid version");
                    pass = false;
                }

                if (tile_validate_sources(lon1, lat1, lon2, lat2, ch.src_files_magic) == false)
                {
                    WARN("Cache was made with different files");
                    pass = false;
                }
            }

            if (pass)
            {
                res = f_read(&f, gui.map.chunks[index].buffer, MAP_BUFFER_SIZE, &br);
                if (res != FR_OK || br != MAP_BUFFER_SIZE)
                {
                    WARN("Cache body size not valid");
                    pass = false;
                }

                for (uint8_t i = 0; i < ch.number_of_poi; i++)
                {
                    cache_poi_t cp;
                    f_read(&f, &cp, sizeof(cache_poi_t), &br);

                    map_poi_t poi;
                    poi.chunk = index;
                    poi.type = cp.type;
                    poi.uid = cp.uid;
                    poi.magic = 0xFF;
                    poi.x = cp.x;
                    poi.y = cp.y;

                    uint16_t name_len = (cp.name_len + 3) & ~3;
                    char name[name_len];
                    f_read(&f, name, name_len, &br);

                    tile_poi_add(&poi, name, cp.name_len);

                }

            }

            f_close(&f);
        }
    }
    else
    {
        pass = false;
    }

    if (pass)
    {
        gui.map.chunks[index].center_lon = c_lon;
        gui.map.chunks[index].center_lat = c_lat;
        gui.map.chunks[index].zoom = zoom;
        gui.map.chunks[index].ready = true;

        gui.map.magic = (gui.map.magic + 1) % 0xFF;

        return true;
    }

    return false;
}


bool tile_generate(uint8_t index, int32_t lon, int32_t lat, uint16_t zoom)
{
    //invalidate
    gui.map.chunks[index].ready = false;
    gui.map.magic = (gui.map.magic + 1) % 0xFF;

    char tile_path[PATH_LEN];
    int32_t c_lon, c_lat;

    tile_get_cache(lon, lat, zoom, &c_lon, &c_lat, tile_path);

    uint32_t start = HAL_GetTick();
    DBG("\n\nGeneating start [%u]", index);

    int32_t step_x;
    int32_t step_y;
    geo_get_steps(c_lat, zoom, &step_x, &step_y);

    //get bbox
    uint32_t map_w = MAP_W * step_x;
    uint32_t map_h = (MAP_H * step_y);
    int32_t lon1 = c_lon - map_w / 2;
    int32_t lon2 = c_lon + map_w / 2;
    int32_t lat1 = c_lat + map_h / 2;
    int32_t lat2 = c_lat - map_h / 2;

    //create tile
    cache_header_t ch = {0};

    ch.start_word = CACHE_START_WORD;
    ch.version = CACHE_VERSION;

    //assign chunk to canvas
    lv_canvas_set_buffer(gui.map.canvas, gui.map.chunks[index].buffer, MAP_W, MAP_H, LV_IMG_CF_TRUE_COLOR);

    //draw topomap
    draw_topo(lon1, lat1, lon2, lat2, step_x, step_y, ch.src_files_magic);

    //draw lines and popygons
    tile_create(lon1, lat1, lon2, lat2, step_x, step_y, zoom, ch.src_files_magic, index);

    DBG("Saving tile to storage");

    ch.number_of_poi = 0;
    for (uint8_t i = 0; i < NUMBER_OF_POI; i++)
    {
        if (gui.map.poi[i].chunk == index)
            ch.number_of_poi++;
    }

    FIL f;
    UINT bw;

    //create dir
    char dir_path[PATH_LEN];
    sprintf(dir_path, PATH_MAP_CACHE_DIR "/%u", zoom);
    f_mkdir(dir_path);

    //write cache
    f_open(&f, tile_path, FA_WRITE | FA_CREATE_ALWAYS);
    f_write(&f, &ch, sizeof(cache_header_t), &bw);
    f_write(&f, gui.map.chunks[index].buffer, MAP_BUFFER_SIZE, &bw);

    //write POIs
    for (uint8_t i = 0; i < NUMBER_OF_POI; i++)
    {
        if (gui.map.poi[i].chunk == index)
        {
            cache_poi_t cp;
            cp.x = gui.map.poi[i].x;
            cp.y = gui.map.poi[i].y;
            cp.type = gui.map.poi[i].type;
            cp.uid = gui.map.poi[i].uid;
            cp.name_len = strlen(gui.map.poi[i].name);

            f_write(&f, &cp, sizeof(cache_poi_t), &bw);
            uint16_t name_len = (cp.name_len + 3) & ~3;
            f_write(&f, gui.map.poi[i].name, name_len, &bw);
        }
    }


    f_close(&f);

    DBG("duration %u ms", HAL_GetTick() - start);

    gui.map.chunks[index].center_lon = c_lon;
    gui.map.chunks[index].center_lat = c_lat;
    gui.map.chunks[index].zoom = zoom;
    gui.map.chunks[index].ready = true;

    gui.map.magic = (gui.map.magic + 1) % 0xFF;

    return true;
}
