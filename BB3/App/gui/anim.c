/*
 * anim.c
 *
 *  Created on: Jan 14, 2021
 *      Author: horinek
 */
#include "anim.h"

static void anim_opa_scale_anim(lv_obj_t * obj, lv_anim_value_t v)
{
    lv_obj_set_style_local_opa_scale(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, v);
}

static void anim_fade_out_delete_ready(lv_anim_t * a)
{
    lv_obj_del(a->var);
}

void anim_fade_out_delete(lv_obj_t * obj)
{
    lv_group_remove_obj(obj);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_values(&a, LV_OPA_COVER, LV_OPA_TRANSP);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)anim_opa_scale_anim);
    lv_anim_set_ready_cb(&a, anim_fade_out_delete_ready);
    lv_anim_start(&a);
}

void anim_fade_in(lv_obj_t * obj)
{
    lv_obj_fade_in(obj, 100, 0);
}
