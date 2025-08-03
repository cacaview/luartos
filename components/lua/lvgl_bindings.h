#ifndef LVGL_BINDINGS_H
#define LVGL_BINDINGS_H

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Register LVGL bindings in Lua state
 * @param L Lua state
 * @return int 0 on success, non-zero on error
 */
int luaopen_lvgl(lua_State* L);

// Helper functions for common LVGL operations
int lvgl_obj_create(lua_State* L);
int lvgl_obj_set_size(lua_State* L);
int lvgl_obj_set_pos(lua_State* L);
int lvgl_obj_align(lua_State* L);
int lvgl_obj_set_style_bg_color(lua_State* L);
int lvgl_obj_clean(lua_State* L);
int lvgl_obj_invalidate(lua_State* L);
int lvgl_obj_center(lua_State* L);
int lvgl_obj_set_style_text_color(lua_State* L);
int lvgl_obj_set_style_text_font(lua_State* L);
int lvgl_obj_set_style_border_width(lua_State* L);
int lvgl_obj_set_style_border_color(lua_State* L);
int lvgl_obj_align_to(lua_State* L);

// Widget creation functions
int lvgl_label_create(lua_State* L);
int lvgl_label_set_text(lua_State* L);
int lvgl_btn_create(lua_State* L);
int lvgl_slider_create(lua_State* L);
int lvgl_slider_set_value(lua_State* L);
int lvgl_switch_create(lua_State* L);
int lvgl_bar_create(lua_State* L);
int lvgl_bar_set_value(lua_State* L);

// Utility functions
int lvgl_scr_act(lua_State* L);
int lvgl_color_hex(lua_State* L);
int lvgl_color_white(lua_State* L);
int lvgl_refr_now(lua_State* L);

// Font constants
int lvgl_font_montserrat_14(lua_State* L);

// Animation constants
int lvgl_anim_off(lua_State* L);

// Alignment constants
int lvgl_align_center(lua_State* L);
int lvgl_align_top_mid(lua_State* L);
int lvgl_align_bottom_mid(lua_State* L);
int lvgl_align_out_top_mid(lua_State* L);

// Part constants
int lvgl_part_main(lua_State* L);

#ifdef __cplusplus
}
#endif

#endif // LVGL_BINDINGS_H
