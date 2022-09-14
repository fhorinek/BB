#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif


#ifndef LV_ATTRIBUTE_MEM_ALIGN
#define LV_ATTRIBUTE_MEM_ALIGN
#endif

#ifndef LV_ATTRIBUTE_IMG_ROCKET_IMG
#define LV_ATTRIBUTE_IMG_ROCKET_IMG
#endif

const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST LV_ATTRIBUTE_IMG_ROCKET_IMG uint8_t rocket_img_map[] = {
  0xc0, 
  0xc0, 
  0xc0, 
  0xc0, 
  0xc0, 
  0xc0, 
};

const lv_img_dsc_t rocket_img = {
  .header.cf = LV_IMG_CF_ALPHA_1BIT,
  .header.always_zero = 0,
  .header.reserved = 0,
  .header.w = 2,
  .header.h = 6,
  .data_size = 6,
  .data = rocket_img_map,
};
