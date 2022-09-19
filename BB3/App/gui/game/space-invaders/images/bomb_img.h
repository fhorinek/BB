#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif


#ifndef LV_ATTRIBUTE_MEM_ALIGN
#define LV_ATTRIBUTE_MEM_ALIGN
#endif

#ifndef LV_ATTRIBUTE_IMG_BOMB_IMG
#define LV_ATTRIBUTE_IMG_BOMB_IMG
#endif

const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST LV_ATTRIBUTE_IMG_BOMB_IMG uint8_t bomb_img_map[] = {
  0x30, 
  0x30, 
  0xc0, 
  0xc0, 
  0x30, 
  0x30, 
  0x0c, 
  0x0c, 
  0x30, 
  0x30, 
};

const lv_img_dsc_t bomb_img = {
  .header.cf = LV_IMG_CF_ALPHA_1BIT,
  .header.always_zero = 0,
  .header.reserved = 0,
  .header.w = 6,
  .header.h = 10,
  .data_size = 10,
  .data = bomb_img_map,
};
