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

#ifndef LV_ATTRIBUTE_IMG_IMG_HOME
#define LV_ATTRIBUTE_IMG_IMG_HOME
#endif

const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST LV_ATTRIBUTE_IMG_IMG_HOME uint8_t img_home_map[] = {
  0x00, 0x20, 0x00, 
  0x00, 0xf0, 0x00, 
  0x01, 0xf8, 0x00, 
  0x03, 0xfc, 0x00, 
  0x07, 0x9e, 0x00, 
  0x0f, 0x6f, 0x00, 
  0x1e, 0xf7, 0x80, 
  0x3d, 0xfb, 0xc0, 
  0x7b, 0xfd, 0xe0, 
  0xf7, 0xfe, 0xf0, 
  0x2f, 0xff, 0x40, 
  0x1f, 0xff, 0x80, 
  0x1f, 0xff, 0x80, 
  0x1f, 0x0f, 0x80, 
  0x1f, 0x0f, 0x80, 
  0x1f, 0x0f, 0x80, 
  0x1f, 0x0f, 0x80, 
  0x1f, 0x0f, 0x80, 
};

const lv_img_dsc_t img_home = {
  .header.cf = LV_IMG_CF_ALPHA_1BIT,
  .header.always_zero = 0,
  .header.reserved = 0,
  .header.w = 20,
  .header.h = 18,
  .data_size = 54,
  .data = img_home_map,
};
