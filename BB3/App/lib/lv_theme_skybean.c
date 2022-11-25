/**
 * @file lv_theme_skybean.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lvgl/lvgl.h" /*To see all the widgets*/

#include "lvgl/src/lv_misc/lv_gc.h"

#if defined(LV_GC_INCLUDE)
    #include LV_GC_INCLUDE
#endif /* LV_ENABLE_GC */

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef struct {
    lv_style_t bg;
    lv_style_t btn;
    lv_style_t round;
    lv_style_t color;
    lv_style_t gray;
    lv_style_t tick_line;
    lv_style_t tight;

    lv_style_t border;

    lv_style_t indicator;
    lv_style_t shadow;
    lv_style_t cursor;
    lv_style_t kb;
    lv_style_t radius;
    lv_style_t white;
    lv_style_t space;
    lv_style_t small_line;

} theme_styles_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void theme_apply(lv_theme_t * th, lv_obj_t * obj, lv_theme_style_t name);

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_theme_t theme;

static lv_theme_t theme;
static theme_styles_t * styles;

static bool inited;

/**********************
 *      MACROS
 **********************/

/**********************
 *   STATIC FUNCTIONS
 **********************/
static void style_init_reset(lv_style_t * style);


/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Initialize the default
 * @param color_primary the primary color of the theme
 * @param color_secondary the secondary color for the theme
 * @param flags ORed flags starting with `LV_THEME_DEF_FLAG_...`
 * @param font_small pointer to a small font
 * @param font_normal pointer to a normal font
 * @param font_subtitle pointer to a large font
 * @param font_title pointer to a extra large font
 * @return a pointer to reference this theme later
 */
lv_theme_t * lv_theme_skybean_init(lv_color_t color_primary, lv_color_t color_secondary, uint32_t flags,
                                    const lv_font_t * font_small, const lv_font_t * font_normal, const lv_font_t * font_subtitle,
                                    const lv_font_t * font_title)
{

    /* This trick is required only to avoid the garbage collection of
     * styles' data if LVGL is used in a binding (e.g. Micropython)
     * In a general case styles could be simple `static lv_style_t my style` variables or allocated directly into `styles`*/
    if(!inited) {
#if defined(LV_GC_INCLUDE)
        LV_GC_ROOT(_lv_theme_skybean_styles) = lv_mem_alloc(sizeof(theme_styles_t));
        styles = (theme_styles_t *)LV_GC_ROOT(_lv_theme_skybean_styles);
#else
        styles = lv_mem_alloc(sizeof(theme_styles_t));
#endif

    }

    theme.color_primary = color_primary;
    theme.color_secondary = color_secondary;
    theme.font_small = font_small;
    theme.font_normal = font_normal;
    theme.font_subtitle = font_subtitle;
    theme.font_title = font_title;
    theme.flags = flags;


    style_init_reset(&styles->bg);
    lv_style_set_bg_opa(&styles->bg, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_bg_color(&styles->bg, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_style_set_line_width(&styles->bg, LV_STATE_DEFAULT, 1);
	lv_style_set_scale_end_line_width(&styles->bg, LV_STATE_DEFAULT, 1);
	lv_style_set_scale_end_color(&styles->bg, LV_STATE_DEFAULT, theme.color_primary);
	lv_style_set_text_color(&styles->bg, LV_STATE_DEFAULT, LV_COLOR_WHITE);

	lv_style_set_pad_left(&styles->bg, LV_STATE_DEFAULT, LV_DPI / 10);
	lv_style_set_pad_right(&styles->bg, LV_STATE_DEFAULT, LV_DPI / 10);
	lv_style_set_pad_top(&styles->bg, LV_STATE_DEFAULT, LV_DPI / 10);
	lv_style_set_pad_bottom(&styles->bg, LV_STATE_DEFAULT, LV_DPI / 10);

    lv_style_set_bg_color(&styles->bg, LV_STATE_FOCUSED, LV_COLOR_WHITE);
    lv_style_set_text_color(&styles->bg, LV_STATE_FOCUSED, LV_COLOR_BLACK);

    lv_style_set_bg_color(&styles->bg, LV_STATE_EDITED, LV_COLOR_BLUE);


    style_init_reset(&styles->btn);
    lv_style_set_bg_color(&styles->btn, LV_STATE_PRESSED, lv_color_hex3(0xccc));
    lv_style_set_bg_color(&styles->btn, LV_STATE_CHECKED, theme.color_primary);
    lv_style_set_bg_color(&styles->btn, LV_STATE_CHECKED | LV_STATE_PRESSED, lv_color_darken(theme.color_primary,
                                                                                             LV_OPA_30));
    lv_style_set_bg_color(&styles->btn, LV_STATE_DISABLED, LV_COLOR_SILVER);
    lv_style_set_text_color(&styles->btn, LV_STATE_DISABLED, LV_COLOR_GRAY);
    lv_style_set_image_recolor(&styles->btn, LV_STATE_DISABLED, LV_COLOR_GRAY);

    style_init_reset(&styles->round);
    lv_style_set_radius(&styles->round, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    lv_style_set_line_rounded(&styles->round, LV_STATE_DEFAULT, true);

    style_init_reset(&styles->color);
    lv_style_set_bg_color(&styles->color, LV_STATE_DEFAULT, theme.color_secondary);
    lv_style_set_line_color(&styles->color, LV_STATE_DEFAULT, theme.color_secondary);

    style_init_reset(&styles->gray);
    lv_style_set_bg_color(&styles->gray, LV_STATE_DEFAULT, LV_COLOR_SILVER);
    lv_style_set_line_color(&styles->gray, LV_STATE_DEFAULT, LV_COLOR_SILVER);
    lv_style_set_text_color(&styles->gray, LV_STATE_DEFAULT, LV_COLOR_GRAY);

    style_init_reset(&styles->tick_line);
    lv_style_set_line_width(&styles->tick_line, LV_STATE_DEFAULT, 5);
    //lv_style_set_scale_end_line_width(&styles->tick_line, LV_STATE_DEFAULT, 5);
    lv_style_set_scale_end_color(&styles->tick_line, LV_STATE_DEFAULT, theme.color_primary);

    style_init_reset(&styles->tight);
    lv_style_set_pad_left(&styles->tight, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_right(&styles->tight, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_top(&styles->tight, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_bottom(&styles->tight, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_inner(&styles->tight, LV_STATE_DEFAULT, 0);

    style_init_reset(&styles->border);
    lv_style_set_border_width(&styles->border, LV_STATE_DEFAULT, 1);
    lv_style_set_border_color(&styles->border, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_style_set_border_width(&styles->border, LV_STATE_CHECKED, 1);
    lv_style_set_border_color(&styles->border, LV_STATE_CHECKED, LV_COLOR_WHITE);

    style_init_reset(&styles->indicator);
    lv_style_set_bg_color(&styles->indicator, LV_STATE_DEFAULT, LV_COLOR_RED);
    lv_style_set_bg_color(&styles->indicator, LV_STATE_CHECKED, LV_COLOR_GREEN);
    lv_style_set_transition_time(&styles->indicator, LV_STATE_DEFAULT, 150);
    lv_style_set_transition_prop_1(&styles->indicator, LV_STATE_DEFAULT, LV_STYLE_BG_COLOR);

    style_init_reset(&styles->shadow);
    lv_style_set_shadow_width(&styles->shadow, LV_STATE_DEFAULT, 20);
    lv_style_set_shadow_color(&styles->shadow, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_style_set_bg_color(&styles->shadow, LV_STATE_DEFAULT, LV_COLOR_WHITE);

    style_init_reset(&styles->cursor);
    lv_style_set_border_width(&styles->cursor, LV_STATE_DEFAULT, 1);
    lv_style_set_border_side(&styles->cursor, LV_STATE_DEFAULT, LV_BORDER_SIDE_LEFT);
    lv_style_set_border_color(&styles->cursor, LV_STATE_DEFAULT, LV_COLOR_WHITE);

    style_init_reset(&styles->kb);
    lv_style_set_bg_opa(&styles->kb, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_bg_color(&styles->kb, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_style_set_bg_color(&styles->kb, LV_STATE_EDITED, LV_COLOR_BLACK);
    lv_style_set_bg_color(&styles->kb, LV_STATE_FOCUSED, LV_COLOR_BLACK);
    lv_style_set_border_side(&styles->kb, LV_STATE_DEFAULT, LV_BORDER_SIDE_TOP);
    lv_style_set_border_width(&styles->kb, LV_STATE_DEFAULT, 2);
    lv_style_set_border_color(&styles->kb, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_style_set_pad_top(&styles->kb, LV_STATE_DEFAULT, 2);

    style_init_reset(&styles->radius);
    lv_style_set_radius(&styles->radius, LV_STATE_DEFAULT, 5);

    style_init_reset(&styles->white);
    lv_style_set_bg_color(&styles->white, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_style_set_bg_opa(&styles->white, LV_STATE_DEFAULT, LV_OPA_COVER);

    style_init_reset(&styles->space);
    lv_style_set_pad_left(&styles->space, LV_STATE_DEFAULT, LV_DPI / 10);
    lv_style_set_pad_right(&styles->space, LV_STATE_DEFAULT, LV_DPI / 10);
    lv_style_set_pad_top(&styles->space, LV_STATE_DEFAULT, LV_DPI / 10);
    lv_style_set_pad_bottom(&styles->space, LV_STATE_DEFAULT, LV_DPI / 10);

    style_init_reset(&styles->small_line);
    lv_style_set_line_width(&styles->small_line, LV_STATE_DEFAULT, 2);
    lv_style_set_line_rounded(&styles->small_line, LV_STATE_DEFAULT, 1);
    lv_style_set_line_color(&styles->small_line, LV_STATE_DEFAULT, LV_COLOR_WHITE);

    theme.apply_xcb = NULL;
    theme.apply_cb = theme_apply;

    return &theme;
}


void theme_apply(lv_theme_t * th, lv_obj_t * obj, lv_theme_style_t name)
{
    LV_UNUSED(th);

    lv_style_list_t * list;

    switch(name) {
        case LV_THEME_NONE:
            break;

        case LV_THEME_SCR:
            list = lv_obj_get_style_list(obj, LV_OBJ_PART_MAIN);
            _lv_style_list_add_style(list, &styles->bg);
            _lv_style_list_add_style(list, &styles->tight);
            break;
        case LV_THEME_OBJ:
            list = lv_obj_get_style_list(obj, LV_OBJ_PART_MAIN);
            _lv_style_list_add_style(list, &styles->bg);
            break;
#if LV_USE_CONT
        case LV_THEME_CONT:
            list = lv_obj_get_style_list(obj, LV_CONT_PART_MAIN);
            _lv_style_list_add_style(list, &styles->bg);
            break;
#endif

#if LV_USE_BTN
        case LV_THEME_BTN:
            list = lv_obj_get_style_list(obj, LV_BTN_PART_MAIN);
            _lv_style_list_add_style(list, &styles->bg);
            _lv_style_list_add_style(list, &styles->btn);
            break;
#endif

#if LV_USE_BTNMATRIX
        case LV_THEME_BTNMATRIX:
            list = lv_obj_get_style_list(obj, LV_BTNMATRIX_PART_BG);
            _lv_style_list_add_style(list, &styles->bg);

            list = lv_obj_get_style_list(obj, LV_BTNMATRIX_PART_BTN);
            _lv_style_list_add_style(list, &styles->bg);
            _lv_style_list_add_style(list, &styles->btn);
            break;
#endif

#if LV_USE_KEYBOARD
        case LV_THEME_KEYBOARD:
            list = lv_obj_get_style_list(obj, LV_KEYBOARD_PART_BG);
//            _lv_style_list_add_style(list, &styles->bg);
            _lv_style_list_add_style(list, &styles->kb);

            list = lv_obj_get_style_list(obj, LV_KEYBOARD_PART_BTN);
            _lv_style_list_add_style(list, &styles->bg);
//            _lv_style_list_add_style(list, &styles->btn);
            break;
#endif

#if LV_USE_BAR
        case LV_THEME_BAR:
            list = lv_obj_get_style_list(obj, LV_BAR_PART_BG);
            _lv_style_list_add_style(list, &styles->bg);
            _lv_style_list_add_style(list, &styles->tight);
            _lv_style_list_add_style(list, &styles->round);

            list = lv_obj_get_style_list(obj, LV_BAR_PART_INDIC);
            _lv_style_list_add_style(list, &styles->bg);
            _lv_style_list_add_style(list, &styles->color);
            break;
#endif

#if LV_USE_SWITCH
        case LV_THEME_SWITCH:
            list = lv_obj_get_style_list(obj, LV_SWITCH_PART_BG);
            _lv_style_list_add_style(list, &styles->bg);
            _lv_style_list_add_style(list, &styles->tight);
            _lv_style_list_add_style(list, &styles->round);
            _lv_style_list_add_style(list, &styles->indicator);

            list = lv_obj_get_style_list(obj, LV_SWITCH_PART_INDIC);

            list = lv_obj_get_style_list(obj, LV_SWITCH_PART_KNOB);
            _lv_style_list_add_style(list, &styles->bg);
            _lv_style_list_add_style(list, &styles->tight);
            _lv_style_list_add_style(list, &styles->round);
            _lv_style_list_add_style(list, &styles->border);
            break;
#endif

#if LV_USE_CANVAS
        case LV_THEME_CANVAS:
            list = lv_obj_get_style_list(obj, LV_CANVAS_PART_MAIN);
            break;
#endif

#if LV_USE_IMG
        case LV_THEME_IMAGE:
            list = lv_obj_get_style_list(obj, LV_IMG_PART_MAIN);
            break;
#endif

#if LV_USE_IMGBTN
        case LV_THEME_IMGBTN:
            list = lv_obj_get_style_list(obj, LV_IMG_PART_MAIN);
            break;
#endif

#if LV_USE_LABEL
        case LV_THEME_LABEL:
            list = lv_obj_get_style_list(obj, LV_LABEL_PART_MAIN);
            break;
#endif

#if LV_USE_LINE
        case LV_THEME_LINE:
            list = lv_obj_get_style_list(obj, LV_LABEL_PART_MAIN);
            _lv_style_list_add_style(list, &styles->small_line);
            break;
#endif

#if LV_USE_ARC
        case LV_THEME_ARC:
            list = lv_obj_get_style_list(obj, LV_ARC_PART_BG);
            _lv_style_list_add_style(list, &styles->bg);
            //_lv_style_list_add_style(list, &styles->tick_line);
            _lv_style_list_add_style(list, &styles->round);

            list = lv_obj_get_style_list(obj, LV_ARC_PART_INDIC);
            _lv_style_list_add_style(list, &styles->bg);
            _lv_style_list_add_style(list, &styles->color);
            _lv_style_list_add_style(list, &styles->tick_line);
            _lv_style_list_add_style(list, &styles->round);
            break;
#endif

#if LV_USE_SPINNER
        case LV_THEME_SPINNER:
            list = lv_obj_get_style_list(obj, LV_SPINNER_PART_BG);
            _lv_style_list_add_style(list, &styles->bg);
            //_lv_style_list_add_style(list, &styles->tick_line);
            _lv_style_list_add_style(list, &styles->round);

            list = lv_obj_get_style_list(obj, LV_SPINNER_PART_INDIC);
            _lv_style_list_add_style(list, &styles->bg);
            _lv_style_list_add_style(list, &styles->color);
            _lv_style_list_add_style(list, &styles->tick_line);
            _lv_style_list_add_style(list, &styles->round);
            break;
#endif

#if LV_USE_SLIDER
        case LV_THEME_SLIDER:
            list = lv_obj_get_style_list(obj, LV_SLIDER_PART_BG);
            _lv_style_list_add_style(list, &styles->bg);
            _lv_style_list_add_style(list, &styles->round);

            list = lv_obj_get_style_list(obj, LV_SLIDER_PART_INDIC);
            _lv_style_list_add_style(list, &styles->bg);
            _lv_style_list_add_style(list, &styles->round);
            _lv_style_list_add_style(list, &styles->color);

            list = lv_obj_get_style_list(obj, LV_SLIDER_PART_KNOB);
            _lv_style_list_add_style(list, &styles->bg);
            _lv_style_list_add_style(list, &styles->round);
            _lv_style_list_add_style(list, &styles->border);
            _lv_style_list_add_style(list, &styles->tight);
            break;
#endif

#if LV_USE_CHECKBOX
        case LV_THEME_CHECKBOX:
            list = lv_obj_get_style_list(obj, LV_CHECKBOX_PART_BG);

            list = lv_obj_get_style_list(obj, LV_CHECKBOX_PART_BULLET);
            _lv_style_list_add_style(list, &styles->bg);
            _lv_style_list_add_style(list, &styles->btn);
            break;
#endif

#if LV_USE_MSGBOX
        case LV_THEME_MSGBOX:
            list = lv_obj_get_style_list(obj, LV_MSGBOX_PART_BG);
            _lv_style_list_add_style(list, &styles->bg);
            break;

        case LV_THEME_MSGBOX_BTNS:
            list = lv_obj_get_style_list(obj, LV_MSGBOX_PART_BTN_BG);
            _lv_style_list_add_style(list, &styles->bg);

            list = lv_obj_get_style_list(obj, LV_MSGBOX_PART_BTN);
            _lv_style_list_add_style(list, &styles->bg);
            _lv_style_list_add_style(list, &styles->btn);
            break;

#endif
#if LV_USE_LED
        case LV_THEME_LED:
            list = lv_obj_get_style_list(obj, LV_LED_PART_MAIN);
            _lv_style_list_add_style(list, &styles->bg);
            _lv_style_list_add_style(list, &styles->round);
            _lv_style_list_add_style(list, &styles->shadow);
            break;
#endif
#if LV_USE_PAGE
        case LV_THEME_PAGE:
            list = lv_obj_get_style_list(obj, LV_PAGE_PART_BG);
            _lv_style_list_add_style(list, &styles->bg);

            list = lv_obj_get_style_list(obj, LV_PAGE_PART_SCROLLABLE);
            _lv_style_list_add_style(list, &styles->bg);

            list = lv_obj_get_style_list(obj, LV_PAGE_PART_SCROLLBAR);
            _lv_style_list_add_style(list, &styles->white);
            break;
#endif
#if LV_USE_TABVIEW
        case LV_THEME_TABVIEW:
            list = lv_obj_get_style_list(obj, LV_TABVIEW_PART_BG);
            _lv_style_list_add_style(list, &styles->bg);

            list = lv_obj_get_style_list(obj, LV_TABVIEW_PART_BG_SCRLLABLE);
            _lv_style_list_add_style(list, &styles->bg);
            _lv_style_list_add_style(list, &styles->color);

            list = lv_obj_get_style_list(obj, LV_TABVIEW_PART_TAB_BG);
            _lv_style_list_add_style(list, &styles->bg);

            list = lv_obj_get_style_list(obj, LV_TABVIEW_PART_INDIC);
            _lv_style_list_add_style(list, &styles->bg);
            _lv_style_list_add_style(list, &styles->color);

            list = lv_obj_get_style_list(obj, LV_TABVIEW_PART_TAB_BTN);
            _lv_style_list_add_style(list, &styles->bg);
            _lv_style_list_add_style(list, &styles->btn);
            break;

        case LV_THEME_TABVIEW_PAGE:
            list = lv_obj_get_style_list(obj, LV_PAGE_PART_BG);
            _lv_style_list_add_style(list, &styles->bg);
            _lv_style_list_add_style(list, &styles->gray);

            list = lv_obj_get_style_list(obj, LV_PAGE_PART_SCROLLABLE);
            _lv_style_list_add_style(list, &styles->bg);

            break;
#endif

#if LV_USE_TILEVIEW
        case LV_THEME_TILEVIEW:
            list = lv_obj_get_style_list(obj, LV_TILEVIEW_PART_BG);
            _lv_style_list_add_style(list, &styles->bg);

            list = lv_obj_get_style_list(obj, LV_TILEVIEW_PART_SCROLLBAR);
            _lv_style_list_add_style(list, &styles->bg);

            list = lv_obj_get_style_list(obj, LV_TILEVIEW_PART_EDGE_FLASH);
            _lv_style_list_add_style(list, &styles->bg);
            break;
#endif


#if LV_USE_ROLLER
        case LV_THEME_ROLLER:
            list = lv_obj_get_style_list(obj, LV_ROLLER_PART_BG);
            _lv_style_list_add_style(list, &styles->space);
            _lv_style_list_add_style(list, &styles->border);
            _lv_style_list_add_style(list, &styles->radius);

            list = lv_obj_get_style_list(obj, LV_ROLLER_PART_SELECTED);
            _lv_style_list_add_style(list, &styles->bg);
            _lv_style_list_add_style(list, &styles->color);
            break;
#endif


#if LV_USE_OBJMASK
        case LV_THEME_OBJMASK:
            list = lv_obj_get_style_list(obj, LV_OBJMASK_PART_MAIN);
            break;
#endif

#if LV_USE_LIST
        case LV_THEME_LIST:
            list = lv_obj_get_style_list(obj, LV_LIST_PART_BG);
            _lv_style_list_add_style(list, &styles->bg);

            list = lv_obj_get_style_list(obj, LV_LIST_PART_SCROLLABLE);

            list = lv_obj_get_style_list(obj, LV_LIST_PART_SCROLLBAR);
            _lv_style_list_add_style(list, &styles->bg);
            break;

        case LV_THEME_LIST_BTN:
            list = lv_obj_get_style_list(obj, LV_BTN_PART_MAIN);
            _lv_style_list_add_style(list, &styles->bg);
            _lv_style_list_add_style(list, &styles->btn);
            break;
#endif

#if LV_USE_DROPDOWN
        case LV_THEME_DROPDOWN:
            list = lv_obj_get_style_list(obj, LV_DROPDOWN_PART_MAIN);
            _lv_style_list_add_style(list, &styles->tight);
            _lv_style_list_add_style(list, &styles->round);

            list = lv_obj_get_style_list(obj, LV_DROPDOWN_PART_LIST);
            _lv_style_list_add_style(list, &styles->bg);
            _lv_style_list_add_style(list, &styles->border);
            _lv_style_list_add_style(list, &styles->radius);

            list = lv_obj_get_style_list(obj, LV_DROPDOWN_PART_SCROLLBAR);
            _lv_style_list_add_style(list, &styles->bg);
            _lv_style_list_add_style(list, &styles->white);
            _lv_style_list_add_style(list, &styles->radius);

            list = lv_obj_get_style_list(obj, LV_DROPDOWN_PART_SELECTED);
            _lv_style_list_add_style(list, &styles->bg);
            _lv_style_list_add_style(list, &styles->color);
            _lv_style_list_add_style(list, &styles->radius);
            break;
#endif

#if LV_USE_CHART
        case LV_THEME_CHART:
            list = lv_obj_get_style_list(obj, LV_CHART_PART_BG);
            _lv_style_list_add_style(list, &styles->bg);

            list = lv_obj_get_style_list(obj, LV_CHART_PART_SERIES_BG);
            _lv_style_list_add_style(list, &styles->bg);

            list = lv_obj_get_style_list(obj, LV_CHART_PART_SERIES);
            _lv_style_list_add_style(list, &styles->bg);
            _lv_style_list_add_style(list, &styles->tight);
            break;
#endif
#if LV_USE_TABLE
        case LV_THEME_TABLE:
            list = lv_obj_get_style_list(obj, LV_TABLE_PART_BG);
            _lv_style_list_add_style(list, &styles->bg);

            list = lv_obj_get_style_list(obj, LV_TABLE_PART_CELL1);
            _lv_style_list_add_style(list, &styles->bg);

            list = lv_obj_get_style_list(obj, LV_TABLE_PART_CELL2);
            _lv_style_list_add_style(list, &styles->bg);

            list = lv_obj_get_style_list(obj, LV_TABLE_PART_CELL3);
            _lv_style_list_add_style(list, &styles->bg);

            list = lv_obj_get_style_list(obj, LV_TABLE_PART_CELL4);
            _lv_style_list_add_style(list, &styles->bg);
            break;
#endif

#if LV_USE_WIN
        case LV_THEME_WIN:
            list = lv_obj_get_style_list(obj, LV_WIN_PART_BG);
            _lv_style_list_add_style(list, &styles->bg);

            list = lv_obj_get_style_list(obj, LV_WIN_PART_SCROLLBAR);
            _lv_style_list_add_style(list, &styles->white);

            list = lv_obj_get_style_list(obj, LV_WIN_PART_CONTENT_SCROLLABLE);
            _lv_style_list_add_style(list, &styles->bg);

            list = lv_obj_get_style_list(obj, LV_WIN_PART_HEADER);
            _lv_style_list_add_style(list, &styles->bg);
            break;

        case LV_THEME_WIN_BTN:
            list = lv_obj_get_style_list(obj, LV_BTN_PART_MAIN);
            _lv_style_list_add_style(list, &styles->bg);
            _lv_style_list_add_style(list, &styles->btn);
            break;
#endif

#if LV_USE_TEXTAREA
        case LV_THEME_TEXTAREA:
            list = lv_obj_get_style_list(obj, LV_TEXTAREA_PART_BG);
            _lv_style_list_add_style(list, &styles->bg);
            _lv_style_list_add_style(list, &styles->border);

            list = lv_obj_get_style_list(obj, LV_TEXTAREA_PART_PLACEHOLDER);
            _lv_style_list_add_style(list, &styles->gray);

            list = lv_obj_get_style_list(obj, LV_TEXTAREA_PART_CURSOR);
            _lv_style_list_add_style(list, &styles->cursor);

            list = lv_obj_get_style_list(obj, LV_TEXTAREA_PART_SCROLLBAR);
            _lv_style_list_add_style(list, &styles->bg);
            break;
#endif


#if LV_USE_SPINBOX
        case LV_THEME_SPINBOX:
            list = lv_obj_get_style_list(obj, LV_SPINBOX_PART_BG);
            _lv_style_list_add_style(list, &styles->bg);

            list = lv_obj_get_style_list(obj, LV_SPINBOX_PART_CURSOR);
            _lv_style_list_add_style(list, &styles->bg);
            break;

        case LV_THEME_SPINBOX_BTN:
            list = lv_obj_get_style_list(obj, LV_BTN_PART_MAIN);
            _lv_style_list_add_style(list, &styles->bg);
            _lv_style_list_add_style(list, &styles->btn);
            break;
#endif

#if LV_USE_CALENDAR
        case LV_THEME_CALENDAR:
            list = lv_obj_get_style_list(obj, LV_CALENDAR_PART_BG);
            _lv_style_list_add_style(list, &styles->bg);

            list = lv_obj_get_style_list(obj, LV_CALENDAR_PART_DATE);
            _lv_style_list_add_style(list, &styles->bg);
            _lv_style_list_add_style(list, &styles->btn);
            _lv_style_list_add_style(list, &styles->tight);

            list = lv_obj_get_style_list(obj, LV_CALENDAR_PART_HEADER);
            _lv_style_list_add_style(list, &styles->bg);

            list = lv_obj_get_style_list(obj, LV_CALENDAR_PART_DAY_NAMES);
            _lv_style_list_add_style(list, &styles->bg);
            _lv_style_list_add_style(list, &styles->tight);
            break;
#endif
#if LV_USE_CPICKER
        case LV_THEME_CPICKER:
            list = lv_obj_get_style_list(obj, LV_CPICKER_PART_MAIN);
            _lv_style_list_add_style(list, &styles->bg);

            list = lv_obj_get_style_list(obj, LV_CPICKER_PART_KNOB);
            _lv_style_list_add_style(list, &styles->bg);
            _lv_style_list_add_style(list, &styles->round);
            break;
#endif

#if LV_USE_LINEMETER
        case LV_THEME_LINEMETER:
            list = lv_obj_get_style_list(obj, LV_LINEMETER_PART_MAIN);
            _lv_style_list_add_style(list, &styles->bg);
            _lv_style_list_add_style(list, &styles->round);
            break;
#endif
#if LV_USE_GAUGE
        case LV_THEME_GAUGE:
            list = lv_obj_get_style_list(obj, LV_GAUGE_PART_MAIN);
            _lv_style_list_add_style(list, &styles->bg);
            _lv_style_list_add_style(list, &styles->round);

            list = lv_obj_get_style_list(obj, LV_GAUGE_PART_MAJOR);
            _lv_style_list_add_style(list, &styles->tick_line);

            list = lv_obj_get_style_list(obj, LV_GAUGE_PART_NEEDLE);
            _lv_style_list_add_style(list, &styles->bg);
            break;
#endif
        default:
            break;
    }

    lv_obj_refresh_style(obj, LV_OBJ_PART_ALL, LV_STYLE_PROP_ALL);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void style_init_reset(lv_style_t * style)
{
    if(inited) lv_style_reset(style);
    else lv_style_init(style);
}
