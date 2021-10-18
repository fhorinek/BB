/*
 * gfx_anim.c
 *
 *  Created on: 23. 2. 2021
 *      Author: horinek
 */
#include "gfx.h"
#include "pwr_mng.h"

typedef struct
{
    int16_t x;
    int16_t y;
} pos_t;

#define PATH_LENGHT 10

typedef pos_t anim_path_t[PATH_LENGHT];

typedef struct
{
    anim_path_t * path;
    bool invert;
    bool flip;
} anim_t;

typedef struct
{
    pos_t offset;
    bool alive;
    anim_t * anim;

    float position;
    float speed;
} gfx_sprite_t;

anim_path_t path_bat =
{
    {20, 416},
    {20, 400},
    {25, 380},
    {33, 366},
    {51, 345},
    {70, 330},
    {85, 315},
    {100, 290},
    {105, 270},
    {105, 250}
};

anim_path_t path_data =
{
    {20, 416},
    {20, 400},
    {28, 380},
    {55, 360},
    {95, 345},
    {145, 345},
    {185, 360},
    {212, 380},
    {220, 400},
    {220, 416}
};

anim_t charger_to_bat =
{
    &path_bat,
    false,
    false
};

anim_t charger_to_data =
{
    &path_data,
    false,
    false
};

anim_t bat_to_data =
{
    &path_bat,
    true,
    true
};

anim_t data_to_bat =
{
    &path_bat,
    false,
    true
};

#define NUMBER_OF_PARTICLES 32
gfx_sprite_t particles[NUMBER_OF_PARTICLES];
uint32_t next_particle = 0;

void gfx_anim_init()
{
    for (uint8_t i = 0; i < NUMBER_OF_PARTICLES; i++)
        particles[i].alive = false;
}

void gfx_anim_get_xy(gfx_sprite_t * sprite, int16_t * x, int16_t * y)
{
    float position;

    if (sprite->anim->invert)
        position = PATH_LENGHT - 1 - sprite->position;
    else
        position = sprite->position;

    pos_t pos1 = (*sprite->anim->path)[(uint8_t)floor(position)];
    pos_t pos2 = (*sprite->anim->path)[(uint8_t)ceil(position)];

    float m = position - floor(position);

    *x = pos1.x + (pos2.x - pos1.x) * m;
    *y = pos1.y + (pos2.y - pos1.y) * m;

    *x += sprite->offset.x;
    *y += sprite->offset.y;

    if (sprite->anim->flip)
        *x = 240 - *x;
}

anim_t * gfx_get_anim(uint8_t gfx_status)
{
    switch (gfx_status)
    {
        case(GFX_STATUS_CHARGE_NONE):
            if (pwr.fuel_gauge.battery_percentage == 100
            		|| pwr.charge_port == PWR_CHARGE_DONE
					|| pwr.charge_port == PWR_CHARGE_WEAK)
                return NULL;
            else
                return &charger_to_bat;

        case(GFX_STATUS_CHARGE_PASS):
        {
            static bool to_data = 0;
            if ((to_data = !to_data))
            {
                return &charger_to_data;
            }
            else
            {
                if (pwr.fuel_gauge.battery_percentage == 100
                		|| pwr.charge_port == PWR_CHARGE_DONE
    					|| pwr.charge_port == PWR_CHARGE_WEAK)
                    return NULL;
                else
                    return &charger_to_bat;
            }
        }

        case(GFX_STATUS_NONE_BOOST):
            if (pwr.boost_output > 0)
                return &bat_to_data;
            else
                return NULL;

        case(GFX_STATUS_CHARGE_DATA):
            return &charger_to_bat;

        case(GFX_STATUS_NONE_DATA):
        case(GFX_STATUS_NONE_CHARGE):
		{
            if (pwr.fuel_gauge.battery_percentage == 100
            		|| pwr.data_port == PWR_DATA_CHARGE_DONE)
                return NULL;
            else
            return &data_to_bat;
        }
    }

    return NULL;
}


bool gfx_anim_step(uint8_t gfx_status)
{
    //add new particle
    if (next_particle < HAL_GetTick())
    {
        for (uint8_t i = 0; i < NUMBER_OF_PARTICLES; i++)
        {
            if (particles[i].alive == false)
            {
                anim_t * anim = gfx_get_anim(gfx_status);
                if (anim != NULL)
                {
                    particles[i].position = 0;
                    particles[i].speed = 0.1 + (random() % 50) / 1000.0;
                    particles[i].alive = true;
                    particles[i].offset.x = (random() % 20) - 10;
                    particles[i].offset.y = (random() % 20) - 10;
                    particles[i].anim = anim;
                }

                if (gfx_status == GFX_STATUS_CHARGE_PASS)
                    next_particle = HAL_GetTick() + 25;
                else
                    next_particle = HAL_GetTick() + 50;
                break;
            }
        }
    }

    static uint32_t last_frame = 0;
    uint32_t delta = HAL_GetTick() - last_frame;
    last_frame = HAL_GetTick();

    float m = delta / 30.0;

    bool done = true;

    //move particle
    for (uint8_t i = 0; i < NUMBER_OF_PARTICLES; i++)
    {
        if (particles[i].alive)
        {
            done = false;

            particles[i].position += particles[i].speed * m;
            if (particles[i].position > PATH_LENGHT - 1)
            {
                particles[i].position = PATH_LENGHT - 1;
                particles[i].alive = false;
            }

            int16_t x, y;
            gfx_anim_get_xy(&particles[i], &x, &y);

            //draw
            gfx_draw_dot(x, y - 100 - GFX_ANIM_TOP);
        }
    }

    return done;

}


