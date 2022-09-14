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
#include "drivers/tft/tft.h"
#include "gui/gui.h"

#define ALIEN_NUM_X 5             // Number of aliens in X direction
#define ALIEN_NUM_Y 5             // Number of aliens in Y direction
#define ALIEN_NUM (ALIEN_NUM_X * ALIEN_NUM_Y)

#define BOMB_NUM 6                // maximum number of aliens bombs

#define STATUSBAR_HEIGHT 30       // Height of the strato status bar at the top
#define SCORE_HEIGHT 22           // Height of the score line

#define TANK_Y (TFT_HEIGHT - STATUSBAR_HEIGHT - tank_img.header.h)

typedef enum  {
    SPACE_INVADER_GAME_STATE_NONE,
    SPACE_INVADER_GAME_STATE_START,
    SPACE_INVADER_GAME_STATE_RUN,
    SPACE_INVADER_GAME_STATE_STOP,
} space_invader_game_state_t;

static space_invader_game_state_t game_state = SPACE_INVADER_GAME_STATE_NONE;

typedef struct game_data {
	lv_obj_t *background;
    int alien_pos_counter;
    int alien_speed_x;
    uint32_t alien_move_next, rocket_move_next, bomb_move_next;
    int alien_speed;
    lv_obj_t *alien[ALIEN_NUM];
    lv_obj_t *tank, *rocket, *mothership;
    int32_t tank_x;
    int32_t rocket_x, rocket_y;
    int16_t score;
    lv_obj_t *bomb[BOMB_NUM];
    int level;
    int lives;
    lv_group_t *group;
    lv_obj_t *score_label;
} game_data_t;

game_data_t *game;

/**
 * Delay the movement of all aliens/bombs/rocket by the given amount of milliseconds.
 *
 * @param delay_ms the milliseconds to wait for movement of aliens/bombs/rocket.
 */
static void delay_aliens(uint32_t delay_ms)
{
    uint32_t now = HAL_GetTick();

    game->bomb_move_next = game->alien_move_next = game->rocket_move_next = now + delay_ms;
}

/**
 * Remove all bombs from the game.
 */
static void clear_bombs()
{
    for (int i = 0; i < BOMB_NUM; i++)
        lv_obj_set_hidden(game->bomb[i], true);
}
/**
 * Check if a image is exploded.
 * @param obj the image object to check
 *
 * @return true if it shows an explosion, false otherwise
 */
static bool is_exploded(lv_obj_t *obj)
{
    lv_img_dsc_t *src = lv_img_get_src(obj);

    return (src == &explosion_img);
}

static void restart_aliens() {
    lv_obj_t *alien;
    lv_img_dsc_t *image;

    for (int x = 0; x < ALIEN_NUM_X; x++) {
        for (int y = 0; y < ALIEN_NUM_Y; y++) {
            alien = game->alien[x + ALIEN_NUM_X * y];
            switch (y) {
            case 0:
                image = &alien_1_img;
                break;
            case 1:
            case 2:
                image = &alien_2_img;
                break;
            default:
                image = &alien_3_img;
                break;
            }
            lv_img_set_src(alien, image);
            lv_obj_set_height(alien, alien_1_img.header.h / 2); // We do have two sprites in the image
            lv_obj_set_pos(alien, x * alien_1_img.header.w * 4 / 2,
                    SCORE_HEIGHT + mothership_img.header.h + y * alien_1_img.header.h / 2 * 2);
            lv_obj_set_hidden(alien, false);
        }
    }
    lv_obj_set_hidden(game->mothership, true);
}

static void create_aliens()
{
    for (int i = 0; i < ALIEN_NUM; i++) {
        game->alien[i] = lv_img_create(game->background, NULL);
    }

    for (int i = 0; i < BOMB_NUM; i++) {
        game->bomb[i] = lv_img_create(game->background, NULL);
        lv_img_set_src(game->bomb[i], &bomb_img);
        lv_obj_set_hidden(game->bomb[i], true);
    }

    game->mothership = lv_img_create(game->background, NULL);
    lv_img_set_src(game->mothership, &mothership_img);
    lv_obj_set_hidden(game->mothership, true);
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
        if (lv_obj_get_hidden(game->bomb[i])) 
        {
        	// This bomb is currently unused -> take it
        	lv_obj_set_pos(game->bomb[i], x, y);
            lv_obj_set_hidden(game->bomb[i], false);
            lv_obj_set_style_local_image_recolor_opa(game->bomb[i], LV_IMG_PART_MAIN,
                    LV_STATE_DEFAULT, LV_OPA_COVER);
            lv_obj_set_style_local_image_recolor(game->bomb[i], LV_IMG_PART_MAIN,
                    LV_STATE_DEFAULT, LV_COLOR_RED);

            break;
        }
    }
}

static void tank_hit()
{
	lv_img_set_src(game->tank, &explosion_img);
	lv_obj_set_hidden(game->rocket, true);
	game->lives--;
	if (game->lives == 0)
		game_state = SPACE_INVADER_GAME_STATE_STOP;
	clear_bombs();
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
    	if (!lv_obj_get_hidden(game->alien[i]))
    		if (!is_exploded(game->alien[i]))
    			return false;
    return true;
}

static void move_aliens() {
    lv_obj_t *alien;
    int alien_speed_y;

    game->alien_pos_counter++;
    if (game->alien_pos_counter >= 40)
    {
    	game->alien_pos_counter = 0;
        game->alien_speed_x = -game->alien_speed_x;
        alien_speed_y = alien_1_img.header.h / 2;
    } else
        alien_speed_y = 0;

    for (int i = 0; i < ALIEN_NUM; i++) {
        alien = game->alien[i];
        if (lv_obj_get_hidden(alien))
            continue;

        if (is_exploded(alien))
        {
            lv_obj_set_hidden(alien, true);
        } else {
            int y;
            lv_img_dsc_t *src = lv_img_get_src(alien);

            // Switch sprite image:
            lv_img_set_offset_y(alien, src->header.h / 2
                            * (game->alien_pos_counter % 2));

            y = lv_obj_get_y(alien) + alien_speed_y;

            lv_color_t color;
            if (y > TFT_HEIGHT / 4 * 3)
                color = LV_COLOR_YELLOW;
            else if (y > TFT_HEIGHT / 4 * 2)
                color = LV_COLOR_PURPLE;
            else if (y > TFT_HEIGHT / 4 * 1)
                color = LV_COLOR_CYAN;
            else
                color = LV_COLOR_GREEN;

            lv_obj_set_style_local_image_recolor_opa(alien, LV_IMG_PART_MAIN,
                    LV_STATE_DEFAULT, LV_OPA_COVER);
            lv_obj_set_style_local_image_recolor(alien, LV_IMG_PART_MAIN,
                    LV_STATE_DEFAULT, color);

            lv_obj_set_pos(alien, lv_obj_get_x(alien) + 2 * game->alien_speed_x, y);

            if ( collides(alien, game->tank) || y > TANK_Y )
            {
            	tank_hit();
                restart_aliens();
            }
        }
    }

    // Drop bombs
    for (int x = 0; x < ALIEN_NUM_X; x++) {
        for (int y = ALIEN_NUM_Y - 1; y >= 0; y--) {
            alien = game->alien[x + ALIEN_NUM_X * y];
            if (!lv_obj_get_hidden(alien)) {
                if (random() % 10000 > 10000 - game->level * 80)
                {
                    drop_bomb(lv_obj_get_x(alien) + lv_obj_get_width(alien) / 2,
                    		  lv_obj_get_y(alien) + lv_obj_get_height(alien));
                }
                break;
            }
        }
    }

    if (!lv_obj_get_hidden(game->mothership))
    {
    	int x;

        if (is_exploded(game->mothership))
        {
            lv_obj_set_hidden(game->mothership, true);
            lv_img_set_src(game->mothership, &mothership_img);
        } else {
			x = lv_obj_get_x(game->mothership) + 5;
			if ( x > lv_obj_get_width(game->background) )
			{
				lv_obj_set_hidden(game->mothership, true);
			} else {
				lv_obj_set_x(game->mothership, x);
			}
        }
    } else {
    	if (random() % 1000 < 10 )
    	{
    		lv_obj_set_hidden(game->mothership, false);
    		lv_obj_set_pos(game->mothership, 0, SCORE_HEIGHT);
    	}
    }
}

static void move_bombs() {
    for (int i = 0; i < BOMB_NUM; i++) {
        if (!lv_obj_get_hidden(game->bomb[i])) {
            int y;

            y = lv_obj_get_y(game->bomb[i]);
            lv_obj_set_y(game->bomb[i], lv_obj_get_y(game->bomb[i]) + 10);
            if (collides(game->bomb[i], game->tank)) {
            	tank_hit();
                break;
            }
            if (y > TANK_Y)
                lv_obj_set_hidden(game->bomb[i], true);
        }
    }
}

static void tank_event_cb(lv_obj_t *obj, lv_event_t event) {
    uint32_t key = 0;
    int shoot = false;

    switch (event) {
    case LV_EVENT_CLICKED:
    	shoot = true;
    	break;
    case LV_EVENT_KEY:
        key = *((uint32_t*) lv_event_get_data());
        if ((key == LV_KEY_RIGHT || key == LV_KEY_HOME) && game->tank_x < TFT_WIDTH - tank_img.header.w)
            game->tank_x += 4;
        if ((key == LV_KEY_LEFT || key == LV_KEY_ESC) && game->tank_x > 0)
            game->tank_x -= 4;
        if (key == LV_KEY_ENTER) {
        	shoot = true;
        }
    }
    if ( shoot && lv_obj_get_hidden(game->rocket)) {
    	game->rocket_x = game->tank_x + tank_img.header.w / 2;
    	game->rocket_y = TANK_Y - tank_img.header.h;

    	lv_obj_set_pos(game->rocket, game->rocket_x, game->rocket_y);
    	lv_obj_set_hidden(game->rocket, false);
    }
}

static void start_next_level()
{
    game->level++;
    game->alien_speed = 300 / game->level;
    game->alien_speed_x = 1;
    game->alien_pos_counter = 0;

    delay_aliens(1500);

    clear_bombs();
    restart_aliens();

    lv_obj_set_hidden(game->rocket, true);
}

/**
 * Called at the beginning of the game to allocate all resources.
 */
static void game_init() {
    game = malloc(sizeof(game_data_t));
    game->level = 0;
    game->lives = 3;
    game->score = 0;

    game->background = lv_obj_create(lv_scr_act(), false);
    lv_obj_set_size(game->background, TFT_WIDTH, TFT_HEIGHT - STATUSBAR_HEIGHT);
    lv_obj_set_pos(game->background, 0, STATUSBAR_HEIGHT);

    create_aliens();

    game->tank_x = TFT_WIDTH / 2;
    game->tank = lv_img_create(game->background, NULL);
    lv_img_set_src(game->tank, &tank_img);
    lv_obj_set_pos(game->tank, game->tank_x, TANK_Y);
    lv_obj_set_style_local_image_recolor_opa(game->tank, LV_IMG_PART_MAIN,
            LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_obj_set_style_local_image_recolor(game->tank, LV_IMG_PART_MAIN,
            LV_STATE_DEFAULT, LV_COLOR_BLUE);
    
    // create input group
    game->group = lv_group_create();
    lv_group_add_obj(game->group, game->tank);
    lv_indev_set_group(gui.input.indev, game->group);
    lv_obj_set_event_cb(game->tank, tank_event_cb);
    //lv_group_focus_obj(game->group);
    lv_group_set_editing(game->group, true);

    game->rocket = lv_img_create(game->background, NULL);
    lv_img_set_src(game->rocket, &rocket_img);
    lv_obj_set_hidden(game->rocket, true);
    lv_obj_set_style_local_image_recolor_opa(game->rocket, LV_IMG_PART_MAIN,
            LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_obj_set_style_local_image_recolor(game->rocket, LV_IMG_PART_MAIN,
            LV_STATE_DEFAULT, LV_COLOR_WHITE);

    game->score_label = lv_label_create(game->background, NULL);
    lv_obj_set_pos(game->score_label, 0, 0);

    start_next_level();
}

/**
 * Called at the end of the game and frees up all resources.
 */
static void game_stop()
{
  // Set input group 
    lv_indev_reset(gui.input.indev, NULL);
    lv_indev_wait_release(gui.input.indev);
    lv_indev_set_group(gui.input.indev, gui.input.group);
    lv_group_remove_all_objs(game->group);

    for (int i = 0; i < ALIEN_NUM; i++)
        lv_obj_del(game->alien[i]);

    for (int i = 0; i < BOMB_NUM; i++)
        lv_obj_del(game->bomb[i]);

    lv_obj_del(game->rocket);
    lv_obj_del(game->tank);
    lv_obj_del(game->score_label);
    lv_obj_del(game->background);

    free(game);
}

static void game_run()
{
    char label[100];
    uint32_t now = HAL_GetTick();

    if (now > game->alien_move_next)
      {
        game->alien_move_next = now + game->alien_speed;
        move_aliens();
        if ( is_exploded(game->tank) )
        	lv_img_set_src(game->tank, &tank_img);
      }

    lv_obj_set_pos(game->tank, game->tank_x, TANK_Y);

    if (!lv_obj_get_hidden(game->rocket)) {
        lv_obj_set_pos(game->rocket, game->rocket_x, game->rocket_y);
        if (now > game->rocket_move_next) {
            game->rocket_move_next = now + 10;
            game->rocket_y -= 15;
            if (game->rocket_y <= 0) {
                lv_obj_set_hidden(game->rocket, true);
            }

            // check for collision between rocket and aliens
            for (int i = 0; i < ALIEN_NUM; i++) {
                lv_obj_t *alien = game->alien[i];
                if (!lv_obj_get_hidden(alien)) {
                    if (collides(alien, game->rocket)) {
                        lv_img_set_src(alien, &explosion_img);
                        lv_obj_set_hidden(game->rocket, true);
                        game->score += 10;
                        game->alien_speed = game->alien_speed * 8 / 10;
                        if ( are_aliens_dead() )
                        	start_next_level();
                        break;
                    }
                }
            }

            // check for collision between rocket and bombs
            for (int i = 0; i < BOMB_NUM; i++) {
                if (!lv_obj_get_hidden(game->bomb[i])) {
                    if (collides(game->bomb[i], game->rocket)) {
                        lv_obj_set_hidden(game->rocket, true);
                        lv_obj_set_hidden(game->bomb[i], true);
                        game->score += 1;
                    }
                }
            }

            // check for collision between rocket and mothership
            if ( collides(game->mothership, game->rocket) )
            {
            	game->score += 200;
                lv_img_set_src(game->mothership, &explosion_img);
            }
        }
    }

    if (now > game->bomb_move_next) 
      {
    	move_bombs();
    	game->bomb_move_next = now + game->alien_speed;
      }

    sprintf(label, "Score: %d    Lives: %d", game->score, game->lives);
    lv_label_set_text(game->score_label, label);
}

/**
 * Start the game by calling this function.
 */
void spaceinvaders_start()
{
  game_state = SPACE_INVADER_GAME_STATE_START;
}

/**
 * This is the game loop, that has to be called as often as possible.
 * Every call will move the game one step further.
 */
void spaceinvaders_loop()
{
    switch (game_state) 
      {
      case SPACE_INVADER_GAME_STATE_NONE:
        break;

    case SPACE_INVADER_GAME_STATE_START:
        game_init();
        game_state = SPACE_INVADER_GAME_STATE_RUN;
        break;

    case SPACE_INVADER_GAME_STATE_RUN:
        game_run();
        break;

    case SPACE_INVADER_GAME_STATE_STOP:
        game_stop();
        game_state = SPACE_INVADER_GAME_STATE_NONE;
        break;
    }
}
