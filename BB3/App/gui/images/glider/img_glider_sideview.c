#ifdef __has_include
    #if __has_include("lvgl.h")
        #ifndef LV_LVGL_H_INCLUDE_SIMPLE
            #define LV_LVGL_H_INCLUDE_SIMPLE
        #endif
    #endif
#endif

#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
    #include "lvgl.h"
#else
    #include "lvgl/lvgl.h"
#endif


#ifndef LV_ATTRIBUTE_MEM_ALIGN
#define LV_ATTRIBUTE_MEM_ALIGN
#endif

#ifndef LV_ATTRIBUTE_IMG_IMG_GLIDER_SIDEVIEW
#define LV_ATTRIBUTE_IMG_IMG_GLIDER_SIDEVIEW
#endif

const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST LV_ATTRIBUTE_IMG_IMG_GLIDER_SIDEVIEW uint8_t img_glider_sideview_map[] = {
  0xc0, 0x00, 0x00, 
  0xf0, 0x00, 0x00, 
  0xfc, 0x00, 0x00, 
  0xff, 0x00, 0x00, 
  0xff, 0xc0, 0x00, 
  0xff, 0xf0, 0x00, 
  0xff, 0xfc, 0x00, 
  0xff, 0xff, 0x00, 
  0xff, 0xff, 0xc0, 
  0xff, 0xff, 0xf0, 
  0xff, 0xff, 0xf8, 
  0xff, 0xff, 0xfc, 
  0x7f, 0xff, 0xf8, 
};

const lv_img_dsc_t img_glider_sideview = {
  .header.cf = LV_IMG_CF_ALPHA_1BIT,
  .header.always_zero = 0,
  .header.reserved = 0,
  .header.w = 22,
  .header.h = 13,
  .data_size = 39,
  .data = img_glider_sideview_map,
};
