/**
 * Space Invaders!
 *
 * 2022-09-13
 */
#include "spaceinvaders.h"
#include <gui/game/space-invaders/images/alien_1_img.h>
#include <gui/game/space-invaders/images/alien_2_img.h>
#include <gui/game/space-invaders/images/alien_3_img.h>
#include <gui/game/space-invaders/images/mothership_img.h>
#include <gui/game/space-invaders/images/bomb_img.h>
#include <gui/game/space-invaders/images/explosion_img.h>
#include <gui/game/space-invaders/images/rocket_img.h>
#include <gui/game/space-invaders/images/tank_img.h>
#include "gui/gui.h"
#include "gui/tasks/menu/development/development.h"

#define ALIEN_NUM_X 5             // Number of aliens in X direction
#define ALIEN_NUM_Y 5             // Number of aliens in Y direction
#define ALIEN_NUM (ALIEN_NUM_X * ALIEN_NUM_Y)

#define BOMB_NUM 6                // maximum number of aliens bombs

#define SCORE_HEIGHT 30           // Height of the score line

REGISTER_TASK_ILS(spaceinvaders,
		lv_obj_t *background;

	    // HAL_GetTick when next move should be made:
	    uint32_t alien_move_next, rocket_move_next, bomb_move_next;

	    int alien_pos_counter;
	    int alien_speed_x;
	    int alien_speed;
	    lv_obj_t *alien[ALIEN_NUM];
	    lv_obj_t *bomb[BOMB_NUM];
	    lv_obj_t *mothership;

	    lv_obj_t *tank;

	    lv_obj_t *rocket;

	    int16_t score;
	    lv_obj_t *score_label;

	    int level;
	    int lives;
);

/**
 * Delay the movement of all aliens/bombs/rocket by the given amount of milliseconds.
 *
 * @param delay_ms the milliseconds to wait for movement of aliens/bombs/rocket.
 */
static void delay_aliens(uint32_t delay_ms)
{
    uint32_t now = HAL_GetTick();

    local->bomb_move_next = local->alien_move_next = local->rocket_move_next = now + delay_ms;
}

/**
 * Remove all bombs from the game.
 */
static void clear_bombs()
{
    for (int i = 0; i < BOMB_NUM; i++)
        lv_obj_set_hidden(local->bomb[i], true);
}
/**
 * Check if a image is exploded.
 * @param obj the image object to check
 *
 * @return true if it shows an explosion, false otherwise
 */
static bool is_exploded(lv_obj_t *obj)
{
    lv_img_dsc_t *src = (lv_img_dsc_t *)lv_img_get_src(obj);

    return (src == &explosion_img);
}

static void restart_aliens() {
    lv_obj_t *alien;
    lv_img_dsc_t *image;

    for (int x = 0; x < ALIEN_NUM_X; x++) {
        for (int y = 0; y < ALIEN_NUM_Y; y++) {
            alien = local->alien[x + ALIEN_NUM_X * y];
            switch (y) {
            case 0:
                image = (lv_img_dsc_t *)&alien_1_img;
                break;
            case 1:
            case 2:
                image = (lv_img_dsc_t *)&alien_2_img;
                break;
            default:
                image = (lv_img_dsc_t *)&alien_3_img;
                break;
            }
            lv_img_set_src(alien, image);
            lv_obj_set_height(alien, alien_1_img.header.h / 2); // We do have two sprites in the image
            lv_obj_set_pos(alien, x * alien_1_img.header.w * 4 / 2,
                    SCORE_HEIGHT + mothership_img.header.h + y * alien_1_img.header.h / 2 * 2);
            lv_obj_set_hidden(alien, false);
        }
    }
    lv_obj_set_hidden(local->mothership, true);
}

static void create_aliens()
{
    for (int i = 0; i < ALIEN_NUM; i++) {
        local->alien[i] = lv_img_create(local->background, NULL);
    }

    for (int i = 0; i < BOMB_NUM; i++) {
        local->bomb[i] = lv_img_create(local->background, NULL);
        lv_img_set_src(local->bomb[i], &bomb_img);
        lv_obj_set_hidden(local->bomb[i], true);
    }

    local->mothership = lv_img_create(local->background, NULL);
    lv_img_set_src(local->mothership, &mothership_img);
    lv_obj_set_hidden(local->mothership, true);
}

/**
 * Check if two images collide.
 *
 * @param obj1 the first object
 * @param obj2 the second object
 *
 * @return true if they overlap/collide, otherwise false
 */
static bool collides(lv_obj_t *obj1, lv_obj_t *obj2)
{
    int o1_x1, o1_y1, o1_x2, o1_y2;
    int o2_x1, o2_y1, o2_x2, o2_y2;

    o1_x1 = lv_obj_get_x(obj1);
    o1_y1 = lv_obj_get_y(obj1);
    o1_x2 = o1_x1 + lv_obj_get_width(obj1);
    o1_y2 = o1_y1 + lv_obj_get_height(obj1);
    o2_x1 = lv_obj_get_x(obj2);
    o2_y1 = lv_obj_get_y(obj2);
    o2_x2 = o2_x1 + lv_obj_get_width(obj2);
    o2_y2 = o2_y1 + lv_obj_get_height(obj2);

    return (o2_x1 < o1_x2 && o1_x1 < o2_x2 && o2_y2 > o1_y1 && o1_y2 > o2_y1);
}

/**
 * Drop a bomb at the given position.
 *
 * @param x the X position
 * @param y the Y position
 */
static void drop_bomb(int x, int y)
{
    for (int i = 0; i < BOMB_NUM; i++)
    {
        if (lv_obj_get_hidden(local->bomb[i]))
        {
        	// This bomb is currently unused -> take it
        	lv_obj_set_pos(local->bomb[i], x, y);
            lv_obj_set_hidden(local->bomb[i], false);
            lv_obj_set_style_local_image_recolor_opa(local->bomb[i], LV_IMG_PART_MAIN,
                    LV_STATE_DEFAULT, LV_OPA_COVER);
            lv_obj_set_style_local_image_recolor(local->bomb[i], LV_IMG_PART_MAIN,
                    LV_STATE_DEFAULT, LV_COLOR_RED);

            break;
        }
    }
}

static void tank_hit()
{
	lv_img_set_src(local->tank, &explosion_img);
	lv_obj_set_hidden(local->rocket, true);
	local->lives--;
	clear_bombs();
	if (local->lives == 0)
	{
		//game end
		gui_switch_task(&gui_development, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
	}
	delay_aliens(1500);
}

/**
 * Check if all aliens are dead.
 *
 * @return true if all aliens are dead, false otherwise
 */
static bool are_aliens_dead()
{
    for (int i = 0; i < ALIEN_NUM; i++)
    	if (!lv_obj_get_hidden(local->alien[i]))
    		if (!is_exploded(local->alien[i]))
    			return false;
    return true;
}

static void move_aliens() {
    lv_obj_t *alien;
    int alien_speed_y;

    local->alien_pos_counter++;
    if (local->alien_pos_counter >= 40)
    {
    	local->alien_pos_counter = 0;
        local->alien_speed_x = -local->alien_speed_x;
        alien_speed_y = alien_1_img.header.h / 2;
    } else
        alien_speed_y = 0;

    for (int i = 0; i < ALIEN_NUM; i++) {
        alien = local->alien[i];
        if (lv_obj_get_hidden(alien))
            continue;

        if (is_exploded(alien))
        {
            lv_obj_set_hidden(alien, true);
        } else {
            int y;
            lv_color_t color;
            lv_coord_t height;

            lv_img_dsc_t *src = (lv_img_dsc_t *)lv_img_get_src(alien);

            // Switch sprite image:
            lv_img_set_offset_y(alien, src->header.h / 2
                            * (local->alien_pos_counter % 2));

            y = lv_obj_get_y(alien) + alien_speed_y;

            height = lv_obj_get_height(local->background);
            if (y > height / 4 * 3)
                color = LV_COLOR_YELLOW;
            else if (y > height / 4 * 2)
                color = LV_COLOR_PURPLE;
            else if (y > height / 4 * 1)
                color = LV_COLOR_CYAN;
            else
                color = LV_COLOR_GREEN;

            lv_obj_set_style_local_image_recolor_opa(alien, LV_IMG_PART_MAIN,
                    LV_STATE_DEFAULT, LV_OPA_COVER);
            lv_obj_set_style_local_image_recolor(alien, LV_IMG_PART_MAIN,
                    LV_STATE_DEFAULT, color);

            lv_obj_set_pos(alien, lv_obj_get_x(alien) + 2 * local->alien_speed_x, y);

            if ( collides(alien, local->tank) || y > lv_obj_get_y(local->tank) )
            {
            	tank_hit();
                restart_aliens();
            }
        }
    }

    // Drop bombs
    for (int x = 0; x < ALIEN_NUM_X; x++) {
        for (int y = ALIEN_NUM_Y - 1; y >= 0; y--) {
            alien = local->alien[x + ALIEN_NUM_X * y];
            if (!lv_obj_get_hidden(alien)) {
                if (random() % 10000 > 10000 - local->level * 80)
                {
                    drop_bomb(lv_obj_get_x(alien) + lv_obj_get_width(alien) / 2,
                    		  lv_obj_get_y(alien) + lv_obj_get_height(alien));
                }
                break;
            }
        }
    }

    if (!lv_obj_get_hidden(local->mothership))
    {
    	int x;

        if (is_exploded(local->mothership))
        {
            lv_obj_set_hidden(local->mothership, true);
            lv_img_set_src(local->mothership, &mothership_img);
        } else {
			x = lv_obj_get_x(local->mothership) + 5;
			if ( x > lv_obj_get_width(local->background) )
			{
				lv_obj_set_hidden(local->mothership, true);
			} else {
				lv_obj_set_x(local->mothership, x);
			}
        }
    } else {
    	if (random() % 1000 < 10 )
    	{
    		lv_obj_set_hidden(local->mothership, false);
    		lv_obj_set_pos(local->mothership, 0, SCORE_HEIGHT);
    	}
    }
}

static void move_bombs() {
    for (int i = 0; i < BOMB_NUM; i++) {
        if (!lv_obj_get_hidden(local->bomb[i])) {
            int y;

            y = lv_obj_get_y(local->bomb[i]);
            lv_obj_set_y(local->bomb[i], lv_obj_get_y(local->bomb[i]) + 10);
            if (collides(local->bomb[i], local->tank)) {
            	tank_hit();
                break;
            }
            if (y > lv_obj_get_y(local->tank) + lv_obj_get_height(local->tank))
                lv_obj_set_hidden(local->bomb[i], true);
        }
    }
}

static void tank_event_cb(lv_obj_t *obj, lv_event_t event) {
    uint32_t key = 0;
    int shoot = false;
    lv_coord_t tank_x;

    tank_x = lv_obj_get_x(local->tank);

    switch (event) {
    case LV_EVENT_CANCEL:
    	gui_switch_task(&gui_development, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
    	break;
    case LV_EVENT_CLICKED:
    	shoot = true;
    	break;
    case LV_EVENT_KEY:
        key = *((uint32_t*) lv_event_get_data());
        if ((key == LV_KEY_RIGHT) && tank_x < lv_obj_get_width(local->background) - tank_img.header.w) {
            tank_x += 4;
            lv_obj_set_x(local->tank, tank_x);
        } else if ((key == LV_KEY_LEFT) && tank_x > 0) {
            tank_x -= 4;
            lv_obj_set_x(local->tank, tank_x);
        }
    }
    if ( shoot && lv_obj_get_hidden(local->rocket)) {
    	lv_coord_t rocket_x = tank_x + tank_img.header.w / 2;
    	lv_coord_t rocket_y = lv_obj_get_y(local->tank) - rocket_img.header.h ;

    	lv_obj_set_pos(local->rocket, rocket_x, rocket_y);
    	lv_obj_set_hidden(local->rocket, false);
    }
}

static void start_next_level()
{
    local->level++;
    local->alien_speed = 300 / local->level;
    local->alien_speed_x = 1;
    local->alien_pos_counter = 0;

    delay_aliens(1500);

    clear_bombs();
    restart_aliens();

    lv_obj_set_hidden(local->rocket, true);
}

/**
 * Called at the beginning of the game to allocate all resources.
 */
static lv_obj_t * spaceinvaders_init(lv_obj_t * par)
{
    local->level = 0;
    local->lives = 3;
    local->score = 0;

    local->background = lv_obj_create(par, false);
    lv_obj_set_size(local->background, lv_obj_get_width(par), lv_obj_get_height(par));
    lv_obj_set_pos(local->background, 0, 0);

    create_aliens();

    local->tank = lv_img_create(local->background, NULL);
    lv_img_set_src(local->tank, &tank_img);
    lv_obj_set_pos(local->tank, lv_obj_get_width(local->background) / 2, lv_obj_get_height(local->background) - tank_img.header.h);
    lv_obj_set_style_local_image_recolor_opa(local->tank, LV_IMG_PART_MAIN,
            LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_obj_set_style_local_image_recolor(local->tank, LV_IMG_PART_MAIN,
            LV_STATE_DEFAULT, LV_COLOR_BLUE);
    
    gui_set_dummy_event_cb(par, tank_event_cb);
    gui_set_loop_period(1);

    local->rocket = lv_img_create(local->background, NULL);
    lv_img_set_src(local->rocket, &rocket_img);
    lv_obj_set_hidden(local->rocket, true);
    lv_obj_set_style_local_image_recolor_opa(local->rocket, LV_IMG_PART_MAIN,
            LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_obj_set_style_local_image_recolor(local->rocket, LV_IMG_PART_MAIN,
            LV_STATE_DEFAULT, LV_COLOR_WHITE);

    local->score_label = lv_label_create(local->background, NULL);
    lv_obj_set_pos(local->score_label, 0, 0);

    start_next_level();

    return local->background;
}

/**
 * Called at the end of the game and frees up all resources.
 */
static void spaceinvaders_stop()
{
    for (int i = 0; i < ALIEN_NUM; i++)
        lv_obj_del(local->alien[i]);

    for (int i = 0; i < BOMB_NUM; i++)
        lv_obj_del(local->bomb[i]);

    lv_obj_del(local->rocket);
    lv_obj_del(local->tank);
    lv_obj_del(local->score_label);
    lv_obj_del(local->background);
}

/**
 * This is the game loop, that has to be called as often as possible.
 * Every call will move the game one step further.
 */
static void spaceinvaders_loop()

{
    char label[100];
    uint32_t now = HAL_GetTick();

    if (now > local->alien_move_next)
      {
        local->alien_move_next = now + local->alien_speed;
        move_aliens();
        if ( is_exploded(local->tank) )
        	lv_img_set_src(local->tank, &tank_img);
      }

    //lv_obj_set_x(local->tank, local->tank_x);

    if (!lv_obj_get_hidden(local->rocket)) {
        if (now > local->rocket_move_next) {
            local->rocket_move_next = now + 10;
            lv_coord_t rocket_y = lv_obj_get_y(local->rocket);
            rocket_y -= 15;
            if (rocket_y <= 0) {
                lv_obj_set_hidden(local->rocket, true);
            } else {
                lv_obj_set_y(local->rocket, rocket_y);
            }

            // check for collision between rocket and aliens
            for (int i = 0; i < ALIEN_NUM; i++) {
                lv_obj_t *alien = local->alien[i];
                if (!lv_obj_get_hidden(alien)) {
                    if (collides(alien, local->rocket)) {
                        lv_img_set_src(alien, &explosion_img);
                        lv_obj_set_hidden(local->rocket, true);
                        local->score += 10;
                        local->alien_speed = local->alien_speed * 8 / 10;
                        if ( are_aliens_dead() )
                        	start_next_level();
                        break;
                    }
                }
            }

            // check for collision between rocket and bombs
            for (int i = 0; i < BOMB_NUM; i++) {
                if (!lv_obj_get_hidden(local->bomb[i])) {
                    if (collides(local->bomb[i], local->rocket)) {
                        lv_obj_set_hidden(local->rocket, true);
                        lv_obj_set_hidden(local->bomb[i], true);
                        local->score += 1;
                    }
                }
            }

            // check for collision between rocket and mothership
            if ( collides(local->mothership, local->rocket) )
            {
            	local->score += 200;
                lv_img_set_src(local->mothership, &explosion_img);
            }
        }
    }

    if (now > local->bomb_move_next)
      {
    	move_bombs();
    	local->bomb_move_next = now + local->alien_speed;
      }

    sprintf(label, "Score: %d    Lives: %d", local->score, local->lives);
    lv_label_set_text(local->score_label, label);
}

