#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif


#ifndef LV_ATTRIBUTE_MEM_ALIGN
#define LV_ATTRIBUTE_MEM_ALIGN
#endif

#ifndef LV_ATTRIBUTE_IMG_TANK_IMG
#define LV_ATTRIBUTE_IMG_TANK_IMG
#endif

const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST LV_ATTRIBUTE_IMG_TANK_IMG uint8_t tank_img_map[] = {
  0x00, 0x30, 0x00, 
  0x00, 0x30, 0x00, 
  0x00, 0xfc, 0x00, 
  0x00, 0xfc, 0x00, 
  0x3f, 0xff, 0xf0, 
  0x3f, 0xff, 0xf0, 
  0xff, 0xff, 0xfc, 
  0xff, 0xff, 0xfc, 
  0xff, 0xff, 0xfc, 
  0xff, 0xff, 0xfc, 
  0xff, 0xff, 0xfc, 
  0xff, 0xff, 0xfc, 
};

const lv_img_dsc_t tank_img = {
  .header.cf = LV_IMG_CF_ALPHA_1BIT,
  .header.always_zero = 0,
  .header.reserved = 0,
  .header.w = 22,
  .header.h = 12,
  .data_size = 36,
  .data = tank_img_map,
};
