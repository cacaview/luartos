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
int lvgl_bar_set_range(lua_State* L);

// Advanced widget functions
int lvgl_spangroup_create(lua_State* L);
int lvgl_spangroup_new_span(lua_State* L);
int lvgl_span_set_text(lua_State* L);
int lvgl_spangroup_set_align(lua_State* L);
int lvgl_spangroup_set_overflow(lua_State* L);
int lvgl_spangroup_set_mode(lua_State* L);
int lvgl_spangroup_refr_mode(lua_State* L);
int lvgl_msgbox_create(lua_State* L);
int lvgl_list_create(lua_State* L);
int lvgl_list_add_text(lua_State* L);
int lvgl_list_add_btn(lua_State* L);

// Style functions
int lvgl_obj_set_style_bg_opa(lua_State* L);
int lvgl_obj_set_style_border_opa(lua_State* L);
int lvgl_obj_set_style_border_side(lua_State* L);
int lvgl_obj_set_style_radius(lua_State* L);
int lvgl_obj_set_style_bg_grad_dir(lua_State* L);
int lvgl_obj_set_style_pad_all(lua_State* L);
int lvgl_obj_set_style_pad_top(lua_State* L);
int lvgl_obj_set_style_pad_bottom(lua_State* L);
int lvgl_obj_set_style_pad_left(lua_State* L);
int lvgl_obj_set_style_pad_right(lua_State* L);
int lvgl_obj_set_style_shadow_width(lua_State* L);
int lvgl_obj_set_style_text_opa(lua_State* L);
int lvgl_obj_set_style_text_letter_space(lua_State* L);
int lvgl_obj_set_style_text_line_space(lua_State* L);
int lvgl_obj_set_style_text_align(lua_State* L);
int lvgl_obj_set_style_text_decor(lua_State* L);
int lvgl_obj_set_style_anim_time(lua_State* L);

// Object configuration functions
int lvgl_obj_set_scrollbar_mode(lua_State* L);
int lvgl_obj_set_width(lua_State* L);
int lvgl_obj_add_style(lua_State* L);
int lvgl_style_init(lua_State* L);
int lvgl_label_set_long_mode(lua_State* L);
int lvgl_bar_set_mode(lua_State* L);
int lvgl_switch_add_state(lua_State* L);

// Event handling
int lvgl_obj_add_event_cb(lua_State* L);

// Utility functions
int lvgl_scr_act(lua_State* L);
int lvgl_color_hex(lua_State* L);
int lvgl_color_white(lua_State* L);
int lvgl_color_black(lua_State* L);
int lvgl_refr_now(lua_State* L);
int lvgl_scr_load_anim(lua_State* L);

// Font constants
int lvgl_font_montserrat_14(lua_State* L);
int lvgl_font_montserrat_16(lua_State* L);
int lvgl_font_montserrat_20(lua_State* L);

// Animation constants
int lvgl_anim_off(lua_State* L);
int lvgl_anim_on(lua_State* L);

// Alignment constants
int lvgl_align_center(lua_State* L);
int lvgl_align_top_left(lua_State* L);
int lvgl_align_top_mid(lua_State* L);
int lvgl_align_top_right(lua_State* L);
int lvgl_align_bottom_left(lua_State* L);
int lvgl_align_bottom_mid(lua_State* L);
int lvgl_align_bottom_right(lua_State* L);
int lvgl_align_out_top_mid(lua_State* L);

// Part constants
int lvgl_part_main(lua_State* L);
int lvgl_part_indicator(lua_State* L);
int lvgl_part_knob(lua_State* L);

// State constants
int lvgl_state_default(lua_State* L);
int lvgl_state_checked(lua_State* L);

// Border constants
int lvgl_border_side_full(lua_State* L);

// Text alignment constants
int lvgl_text_align_left(lua_State* L);
int lvgl_text_align_center(lua_State* L);
int lvgl_text_align_right(lua_State* L);

// Scrollbar mode constants
int lvgl_scrollbar_mode_off(lua_State* L);

// Label mode constants
int lvgl_label_long_wrap(lua_State* L);

// Span constants
int lvgl_span_overflow_clip(lua_State* L);
int lvgl_span_mode_break(lua_State* L);

// Bar mode constants
int lvgl_bar_mode_normal(lua_State* L);

// Grad dir constants
int lvgl_grad_dir_none(lua_State* L);

// Text decoration constants
int lvgl_text_decor_none(lua_State* L);

// Screen load animation constants
int lvgl_scr_load_anim_none(lua_State* L);

#ifdef __cplusplus
}
#endif

#endif // LVGL_BINDINGS_H
