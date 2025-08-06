#include "lvgl_bindings.h"
#include "esp_log.h"

static const char *TAG = "LVGL_BINDINGS";

// Metatable names for LVGL objects
#define LVGL_OBJ_METATABLE "lvgl.obj"

// Helper function to check if a value is a valid LVGL object pointer
static lv_obj_t* check_lvgl_obj(lua_State* L, int index) {
    void** obj_ptr = (void**)luaL_checkudata(L, index, LVGL_OBJ_METATABLE);
    luaL_argcheck(L, obj_ptr != NULL, index, "expected lvgl object");
    lv_obj_t* obj = (lv_obj_t*)*obj_ptr;
    luaL_argcheck(L, obj != NULL, index, "invalid lvgl object (null pointer)");
    return obj;
}

// Helper function to push an LVGL object to Lua stack
static void push_lvgl_obj(lua_State* L, lv_obj_t* obj) {
    if (obj == NULL) {
        lua_pushnil(L);
        return;
    }
    
    void** obj_ptr = (void**)lua_newuserdata(L, sizeof(void*));
    *obj_ptr = obj;
    luaL_getmetatable(L, LVGL_OBJ_METATABLE);
    lua_setmetatable(L, -2);
}

// Object creation and manipulation functions
int lvgl_scr_act(lua_State* L) {
    lv_obj_t* scr = lv_scr_act();
    push_lvgl_obj(L, scr);
    return 1;
}

int lvgl_obj_create(lua_State* L) {
    lv_obj_t* parent = NULL;
    if (lua_gettop(L) > 0 && !lua_isnil(L, 1)) {
        parent = check_lvgl_obj(L, 1);
    }
    
    lv_obj_t* obj = lv_obj_create(parent);
    push_lvgl_obj(L, obj);
    return 1;
}

int lvgl_obj_set_size(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_coord_t width = luaL_checkinteger(L, 2);
    lv_coord_t height = luaL_checkinteger(L, 3);
    
    lv_obj_set_size(obj, width, height);
    return 0;
}

int lvgl_obj_set_pos(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_coord_t x = luaL_checkinteger(L, 2);
    lv_coord_t y = luaL_checkinteger(L, 3);
    
    lv_obj_set_pos(obj, x, y);
    return 0;
}

int lvgl_obj_align(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_align_t align = luaL_checkinteger(L, 2);
    lv_coord_t x_ofs = luaL_checkinteger(L, 3);
    lv_coord_t y_ofs = luaL_checkinteger(L, 4);
    
    lv_obj_align(obj, align, x_ofs, y_ofs);
    return 0;
}

int lvgl_obj_align_to(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_obj_t* base = check_lvgl_obj(L, 2);
    lv_align_t align = luaL_checkinteger(L, 3);
    lv_coord_t x_ofs = luaL_checkinteger(L, 4);
    lv_coord_t y_ofs = luaL_checkinteger(L, 5);
    
    lv_obj_align_to(obj, base, align, x_ofs, y_ofs);
    return 0;
}

int lvgl_obj_center(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_obj_center(obj);
    return 0;
}

int lvgl_obj_clean(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_obj_clean(obj);
    return 0;
}

int lvgl_obj_invalidate(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_obj_invalidate(obj);
    return 0;
}

int lvgl_obj_set_style_bg_color(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    uint32_t color_value = luaL_checkinteger(L, 2);
    lv_style_selector_t selector = luaL_checkinteger(L, 3);
    
    lv_color_t color = lv_color_hex(color_value);
    lv_obj_set_style_bg_color(obj, color, selector);
    return 0;
}

int lvgl_obj_set_style_text_color(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    uint32_t color_value = luaL_checkinteger(L, 2);
    lv_style_selector_t selector = luaL_checkinteger(L, 3);
    
    lv_color_t color = lv_color_hex(color_value);
    lv_obj_set_style_text_color(obj, color, selector);
    return 0;
}

int lvgl_obj_set_style_text_font(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    // For simplicity, we'll use a pointer passed as light userdata
    const lv_font_t* font = (const lv_font_t*)lua_touserdata(L, 2);
    lv_style_selector_t selector = luaL_checkinteger(L, 3);
    
    lv_obj_set_style_text_font(obj, font, selector);
    return 0;
}

int lvgl_obj_set_style_border_width(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_coord_t width = luaL_checkinteger(L, 2);
    lv_style_selector_t selector = luaL_checkinteger(L, 3);
    
    lv_obj_set_style_border_width(obj, width, selector);
    return 0;
}

int lvgl_obj_set_style_border_color(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    uint32_t color_value = luaL_checkinteger(L, 2);
    lv_style_selector_t selector = luaL_checkinteger(L, 3);
    
    lv_color_t color = lv_color_hex(color_value);
    lv_obj_set_style_border_color(obj, color, selector);
    return 0;
}

// Widget creation functions
int lvgl_label_create(lua_State* L) {
    lv_obj_t* parent = check_lvgl_obj(L, 1);
    lv_obj_t* label = lv_label_create(parent);
    push_lvgl_obj(L, label);
    return 1;
}

int lvgl_label_set_text(lua_State* L) {
    lv_obj_t* label = check_lvgl_obj(L, 1);
    const char* text = luaL_checkstring(L, 2);
    
    lv_label_set_text(label, text);
    return 0;
}

int lvgl_btn_create(lua_State* L) {
    lv_obj_t* parent = check_lvgl_obj(L, 1);
    lv_obj_t* btn = lv_btn_create(parent);
    push_lvgl_obj(L, btn);
    return 1;
}

int lvgl_slider_create(lua_State* L) {
    lv_obj_t* parent = check_lvgl_obj(L, 1);
    lv_obj_t* slider = lv_slider_create(parent);
    push_lvgl_obj(L, slider);
    return 1;
}

int lvgl_slider_set_value(lua_State* L) {
    lv_obj_t* slider = check_lvgl_obj(L, 1);
    int32_t value = luaL_checkinteger(L, 2);
    lv_anim_enable_t anim = luaL_checkinteger(L, 3);
    
    lv_slider_set_value(slider, value, anim);
    return 0;
}

int lvgl_switch_create(lua_State* L) {
    lv_obj_t* parent = check_lvgl_obj(L, 1);
    lv_obj_t* sw = lv_switch_create(parent);
    push_lvgl_obj(L, sw);
    return 1;
}

int lvgl_bar_create(lua_State* L) {
    lv_obj_t* parent = check_lvgl_obj(L, 1);
    lv_obj_t* bar = lv_bar_create(parent);
    push_lvgl_obj(L, bar);
    return 1;
}

int lvgl_bar_set_value(lua_State* L) {
    lv_obj_t* bar = check_lvgl_obj(L, 1);
    int32_t value = luaL_checkinteger(L, 2);
    lv_anim_enable_t anim = luaL_checkinteger(L, 3);
    
    lv_bar_set_value(bar, value, anim);
    return 0;
}

int lvgl_bar_set_range(lua_State* L) {
    lv_obj_t* bar = check_lvgl_obj(L, 1);
    int32_t min = luaL_checkinteger(L, 2);
    int32_t max = luaL_checkinteger(L, 3);
    
    lv_bar_set_range(bar, min, max);
    return 0;
}

int lvgl_bar_set_mode(lua_State* L) {
    lv_obj_t* bar = check_lvgl_obj(L, 1);
    lv_bar_mode_t mode = luaL_checkinteger(L, 2);
    
    lv_bar_set_mode(bar, mode);
    return 0;
}

// Advanced widget functions
int lvgl_spangroup_create(lua_State* L) {
    lv_obj_t* parent = check_lvgl_obj(L, 1);
    lv_obj_t* spangroup = lv_spangroup_create(parent);
    push_lvgl_obj(L, spangroup);
    return 1;
}

int lvgl_spangroup_new_span(lua_State* L) {
    lv_obj_t* spangroup = check_lvgl_obj(L, 1);
    lv_span_t* span = lv_spangroup_new_span(spangroup);
    lua_pushlightuserdata(L, span);
    return 1;
}

int lvgl_span_set_text(lua_State* L) {
    lv_span_t* span = (lv_span_t*)lua_touserdata(L, 1);
    const char* text = luaL_checkstring(L, 2);
    
    lv_span_set_text(span, text);
    return 0;
}

int lvgl_spangroup_set_align(lua_State* L) {
    lv_obj_t* spangroup = check_lvgl_obj(L, 1);
    lv_text_align_t align = luaL_checkinteger(L, 2);
    
    lv_spangroup_set_align(spangroup, align);
    return 0;
}

int lvgl_spangroup_set_overflow(lua_State* L) {
    lv_obj_t* spangroup = check_lvgl_obj(L, 1);
    lv_span_overflow_t overflow = luaL_checkinteger(L, 2);
    
    lv_spangroup_set_overflow(spangroup, overflow);
    return 0;
}

int lvgl_spangroup_set_mode(lua_State* L) {
    lv_obj_t* spangroup = check_lvgl_obj(L, 1);
    lv_span_mode_t mode = luaL_checkinteger(L, 2);
    
    lv_spangroup_set_mode(spangroup, mode);
    return 0;
}

int lvgl_spangroup_refr_mode(lua_State* L) {
    lv_obj_t* spangroup = check_lvgl_obj(L, 1);
    lv_spangroup_refr_mode(spangroup);
    return 0;
}

int lvgl_msgbox_create(lua_State* L) {
    lv_obj_t* parent = check_lvgl_obj(L, 1);
    const char* title = luaL_checkstring(L, 2);
    const char* txt = luaL_checkstring(L, 3);
    const char* btn_txts[] = {luaL_checkstring(L, 4), NULL};
    bool add_close_btn = lua_toboolean(L, 5);
    
    lv_obj_t* msgbox = lv_msgbox_create(parent, title, txt, btn_txts, add_close_btn);
    push_lvgl_obj(L, msgbox);
    return 1;
}

int lvgl_list_create(lua_State* L) {
    lv_obj_t* parent = check_lvgl_obj(L, 1);
    lv_obj_t* list = lv_list_create(parent);
    push_lvgl_obj(L, list);
    return 1;
}

int lvgl_list_add_text(lua_State* L) {
    lv_obj_t* list = check_lvgl_obj(L, 1);
    const char* txt = luaL_checkstring(L, 2);
    
    lv_obj_t* txt_obj = lv_list_add_text(list, txt);
    push_lvgl_obj(L, txt_obj);
    return 1;
}

int lvgl_list_add_btn(lua_State* L) {
    lv_obj_t* list = check_lvgl_obj(L, 1);
    const void* icon = lua_touserdata(L, 2); // Can be NULL
    const char* txt = luaL_checkstring(L, 3);
    
    lv_obj_t* btn = lv_list_add_btn(list, icon, txt);
    push_lvgl_obj(L, btn);
    return 1;
}

// Style functions
int lvgl_obj_set_style_bg_opa(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_opa_t opa = luaL_checkinteger(L, 2);
    lv_style_selector_t selector = luaL_checkinteger(L, 3);
    
    lv_obj_set_style_bg_opa(obj, opa, selector);
    return 0;
}

int lvgl_obj_set_style_border_opa(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_opa_t opa = luaL_checkinteger(L, 2);
    lv_style_selector_t selector = luaL_checkinteger(L, 3);
    
    lv_obj_set_style_border_opa(obj, opa, selector);
    return 0;
}

int lvgl_obj_set_style_border_side(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_border_side_t side = luaL_checkinteger(L, 2);
    lv_style_selector_t selector = luaL_checkinteger(L, 3);
    
    lv_obj_set_style_border_side(obj, side, selector);
    return 0;
}

int lvgl_obj_set_style_radius(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_coord_t radius = luaL_checkinteger(L, 2);
    lv_style_selector_t selector = luaL_checkinteger(L, 3);
    
    lv_obj_set_style_radius(obj, radius, selector);
    return 0;
}

int lvgl_obj_set_style_bg_grad_dir(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_grad_dir_t grad_dir = luaL_checkinteger(L, 2);
    lv_style_selector_t selector = luaL_checkinteger(L, 3);
    
    lv_obj_set_style_bg_grad_dir(obj, grad_dir, selector);
    return 0;
}

int lvgl_obj_set_style_pad_all(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_coord_t pad = luaL_checkinteger(L, 2);
    lv_style_selector_t selector = luaL_checkinteger(L, 3);
    
    lv_obj_set_style_pad_all(obj, pad, selector);
    return 0;
}

int lvgl_obj_set_style_pad_top(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_coord_t pad = luaL_checkinteger(L, 2);
    lv_style_selector_t selector = luaL_checkinteger(L, 3);
    
    lv_obj_set_style_pad_top(obj, pad, selector);
    return 0;
}

int lvgl_obj_set_style_pad_bottom(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_coord_t pad = luaL_checkinteger(L, 2);
    lv_style_selector_t selector = luaL_checkinteger(L, 3);
    
    lv_obj_set_style_pad_bottom(obj, pad, selector);
    return 0;
}

int lvgl_obj_set_style_pad_left(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_coord_t pad = luaL_checkinteger(L, 2);
    lv_style_selector_t selector = luaL_checkinteger(L, 3);
    
    lv_obj_set_style_pad_left(obj, pad, selector);
    return 0;
}

int lvgl_obj_set_style_pad_right(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_coord_t pad = luaL_checkinteger(L, 2);
    lv_style_selector_t selector = luaL_checkinteger(L, 3);
    
    lv_obj_set_style_pad_right(obj, pad, selector);
    return 0;
}

int lvgl_obj_set_style_shadow_width(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_coord_t width = luaL_checkinteger(L, 2);
    lv_style_selector_t selector = luaL_checkinteger(L, 3);
    
    lv_obj_set_style_shadow_width(obj, width, selector);
    return 0;
}

int lvgl_obj_set_style_text_opa(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_opa_t opa = luaL_checkinteger(L, 2);
    lv_style_selector_t selector = luaL_checkinteger(L, 3);
    
    lv_obj_set_style_text_opa(obj, opa, selector);
    return 0;
}

int lvgl_obj_set_style_text_letter_space(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_coord_t space = luaL_checkinteger(L, 2);
    lv_style_selector_t selector = luaL_checkinteger(L, 3);
    
    lv_obj_set_style_text_letter_space(obj, space, selector);
    return 0;
}

int lvgl_obj_set_style_text_line_space(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_coord_t space = luaL_checkinteger(L, 2);
    lv_style_selector_t selector = luaL_checkinteger(L, 3);
    
    lv_obj_set_style_text_line_space(obj, space, selector);
    return 0;
}

int lvgl_obj_set_style_text_align(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_text_align_t align = luaL_checkinteger(L, 2);
    lv_style_selector_t selector = luaL_checkinteger(L, 3);
    
    lv_obj_set_style_text_align(obj, align, selector);
    return 0;
}

int lvgl_obj_set_style_text_decor(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_text_decor_t decor = luaL_checkinteger(L, 2);
    lv_style_selector_t selector = luaL_checkinteger(L, 3);
    
    lv_obj_set_style_text_decor(obj, decor, selector);
    return 0;
}

int lvgl_obj_set_style_anim_time(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    uint32_t time = luaL_checkinteger(L, 2);
    lv_style_selector_t selector = luaL_checkinteger(L, 3);
    
    lv_obj_set_style_anim_time(obj, time, selector);
    return 0;
}

// Object configuration functions
int lvgl_obj_set_scrollbar_mode(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_scrollbar_mode_t mode = luaL_checkinteger(L, 2);
    
    lv_obj_set_scrollbar_mode(obj, mode);
    return 0;
}

int lvgl_obj_set_width(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lv_coord_t width = luaL_checkinteger(L, 2);
    
    lv_obj_set_width(obj, width);
    return 0;
}

int lvgl_label_set_long_mode(lua_State* L) {
    lv_obj_t* label = check_lvgl_obj(L, 1);
    lv_label_long_mode_t mode = luaL_checkinteger(L, 2);
    
    lv_label_set_long_mode(label, mode);
    return 0;
}

// Event handling
static lua_State* g_event_lua_state = NULL;
static int g_event_callback_ref = LUA_NOREF;

static void lua_event_callback(lv_event_t* e) {
    if (g_event_lua_state && g_event_callback_ref != LUA_NOREF) {
        lua_rawgeti(g_event_lua_state, LUA_REGISTRYINDEX, g_event_callback_ref);
        if (lua_isfunction(g_event_lua_state, -1)) {
            lua_pushlightuserdata(g_event_lua_state, e);
            lua_call(g_event_lua_state, 1, 0);
        } else {
            lua_pop(g_event_lua_state, 1);
        }
    }
}

int lvgl_obj_add_event_cb(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    
    if (lua_isfunction(L, 2)) {
        g_event_lua_state = L;
        g_event_callback_ref = luaL_ref(L, LUA_REGISTRYINDEX);
        lv_obj_add_event_cb(obj, lua_event_callback, LV_EVENT_ALL, NULL);
    }
    
    return 0;
}

// Screen loading
int lvgl_scr_load_anim(lua_State* L) {
    lv_obj_t* scr = check_lvgl_obj(L, 1);
    lv_scr_load_anim_t anim_type = luaL_checkinteger(L, 2);
    uint32_t time = luaL_checkinteger(L, 3);
    uint32_t delay = luaL_checkinteger(L, 4);
    bool auto_del = lua_toboolean(L, 5);
    
    lv_scr_load_anim(scr, anim_type, time, delay, auto_del);
    return 0;
}

// Utility functions
int lvgl_color_hex(lua_State* L) {
    uint32_t hex_value = luaL_checkinteger(L, 1);
    lua_pushinteger(L, hex_value);
    return 1;
}

int lvgl_color_white(lua_State* L) {
    lua_pushinteger(L, 0xFFFFFF);
    return 1;
}

int lvgl_color_black(lua_State* L) {
    lua_pushinteger(L, 0x000000);
    return 1;
}

int lvgl_refr_now(lua_State* L) {
    lv_refr_now(NULL);
    return 0;
}

// Part constants
int lvgl_part_main(lua_State* L) {
    lua_pushinteger(L, LV_PART_MAIN);
    return 1;
}

int lvgl_part_indicator(lua_State* L) {
    lua_pushinteger(L, LV_PART_INDICATOR);
    return 1;
}

int lvgl_part_knob(lua_State* L) {
    lua_pushinteger(L, LV_PART_KNOB);
    return 1;
}

// State constants
int lvgl_state_default(lua_State* L) {
    lua_pushinteger(L, LV_STATE_DEFAULT);
    return 1;
}

int lvgl_state_checked(lua_State* L) {
    lua_pushinteger(L, LV_STATE_CHECKED);
    return 1;
}

// Border constants
int lvgl_border_side_full(lua_State* L) {
    lua_pushinteger(L, LV_BORDER_SIDE_FULL);
    return 1;
}

// Text alignment constants
int lvgl_text_align_left(lua_State* L) {
    lua_pushinteger(L, LV_TEXT_ALIGN_LEFT);
    return 1;
}

int lvgl_text_align_center(lua_State* L) {
    lua_pushinteger(L, LV_TEXT_ALIGN_CENTER);
    return 1;
}

int lvgl_text_align_right(lua_State* L) {
    lua_pushinteger(L, LV_TEXT_ALIGN_RIGHT);
    return 1;
}

// Scrollbar mode constants
int lvgl_scrollbar_mode_off(lua_State* L) {
    lua_pushinteger(L, LV_SCROLLBAR_MODE_OFF);
    return 1;
}

// Label mode constants
int lvgl_label_long_wrap(lua_State* L) {
    lua_pushinteger(L, LV_LABEL_LONG_WRAP);
    return 1;
}

// Span constants
int lvgl_span_overflow_clip(lua_State* L) {
    lua_pushinteger(L, LV_SPAN_OVERFLOW_CLIP);
    return 1;
}

int lvgl_span_mode_break(lua_State* L) {
    lua_pushinteger(L, LV_SPAN_MODE_BREAK);
    return 1;
}

// Bar mode constants
int lvgl_bar_mode_normal(lua_State* L) {
    lua_pushinteger(L, LV_BAR_MODE_NORMAL);
    return 1;
}

// Grad dir constants
int lvgl_grad_dir_none(lua_State* L) {
    lua_pushinteger(L, LV_GRAD_DIR_NONE);
    return 1;
}

// Text decoration constants
int lvgl_text_decor_none(lua_State* L) {
    lua_pushinteger(L, LV_TEXT_DECOR_NONE);
    return 1;
}

// Screen load animation constants
int lvgl_scr_load_anim_none(lua_State* L) {
    lua_pushinteger(L, LV_SCR_LOAD_ANIM_NONE);
    return 1;
}

// Font constants
int lvgl_font_montserrat_14(lua_State* L) {
    lua_pushlightuserdata(L, (void*)&lv_font_montserrat_14);
    return 1;
}

int lvgl_font_montserrat_16(lua_State* L) {
    lua_pushlightuserdata(L, (void*)&lv_font_montserrat_16);
    return 1;
}

int lvgl_font_montserrat_20(lua_State* L) {
    // Fallback to montserrat_16 since montserrat_20 is not enabled
    lua_pushlightuserdata(L, (void*)&lv_font_montserrat_16);
    return 1;
}

// Animation constants
int lvgl_anim_off(lua_State* L) {
    lua_pushinteger(L, LV_ANIM_OFF);
    return 1;
}

int lvgl_anim_on(lua_State* L) {
    lua_pushinteger(L, LV_ANIM_ON);
    return 1;
}

// Alignment constants
int lvgl_align_center(lua_State* L) {
    lua_pushinteger(L, LV_ALIGN_CENTER);
    return 1;
}

int lvgl_align_top_left(lua_State* L) {
    lua_pushinteger(L, LV_ALIGN_TOP_LEFT);
    return 1;
}

int lvgl_align_top_mid(lua_State* L) {
    lua_pushinteger(L, LV_ALIGN_TOP_MID);
    return 1;
}

int lvgl_align_top_right(lua_State* L) {
    lua_pushinteger(L, LV_ALIGN_TOP_RIGHT);
    return 1;
}

int lvgl_align_bottom_left(lua_State* L) {
    lua_pushinteger(L, LV_ALIGN_BOTTOM_LEFT);
    return 1;
}

int lvgl_align_bottom_mid(lua_State* L) {
    lua_pushinteger(L, LV_ALIGN_BOTTOM_MID);
    return 1;
}

int lvgl_align_bottom_right(lua_State* L) {
    lua_pushinteger(L, LV_ALIGN_BOTTOM_RIGHT);
    return 1;
}

int lvgl_align_out_top_mid(lua_State* L) {
    lua_pushinteger(L, LV_ALIGN_OUT_TOP_MID);
    return 1;
}

// Object metatable functions
static int lvgl_obj_gc(lua_State* L) {
    // LVGL objects are managed by LVGL itself, so we don't free them here
    return 0;
}

static int lvgl_obj_tostring(lua_State* L) {
    lv_obj_t* obj = check_lvgl_obj(L, 1);
    lua_pushfstring(L, "lvgl.obj: %p", obj);
    return 1;
}

// Function registry
static const luaL_Reg lvgl_functions[] = {
    // Core functions
    {"scr_act", lvgl_scr_act},
    {"obj_create", lvgl_obj_create},
    {"obj_set_size", lvgl_obj_set_size},
    {"obj_set_pos", lvgl_obj_set_pos},
    {"obj_align", lvgl_obj_align},
    {"obj_align_to", lvgl_obj_align_to},
    {"obj_center", lvgl_obj_center},
    {"obj_clean", lvgl_obj_clean},
    {"obj_invalidate", lvgl_obj_invalidate},
    {"obj_set_scrollbar_mode", lvgl_obj_set_scrollbar_mode},
    {"obj_set_width", lvgl_obj_set_width},
    {"obj_add_event_cb", lvgl_obj_add_event_cb},
    
    // Style functions
    {"obj_set_style_bg_color", lvgl_obj_set_style_bg_color},
    {"obj_set_style_text_color", lvgl_obj_set_style_text_color},
    {"obj_set_style_text_font", lvgl_obj_set_style_text_font},
    {"obj_set_style_border_width", lvgl_obj_set_style_border_width},
    {"obj_set_style_border_color", lvgl_obj_set_style_border_color},
    {"obj_set_style_bg_opa", lvgl_obj_set_style_bg_opa},
    {"obj_set_style_border_opa", lvgl_obj_set_style_border_opa},
    {"obj_set_style_border_side", lvgl_obj_set_style_border_side},
    {"obj_set_style_radius", lvgl_obj_set_style_radius},
    {"obj_set_style_bg_grad_dir", lvgl_obj_set_style_bg_grad_dir},
    {"obj_set_style_pad_all", lvgl_obj_set_style_pad_all},
    {"obj_set_style_pad_top", lvgl_obj_set_style_pad_top},
    {"obj_set_style_pad_bottom", lvgl_obj_set_style_pad_bottom},
    {"obj_set_style_pad_left", lvgl_obj_set_style_pad_left},
    {"obj_set_style_pad_right", lvgl_obj_set_style_pad_right},
    {"obj_set_style_shadow_width", lvgl_obj_set_style_shadow_width},
    {"obj_set_style_text_opa", lvgl_obj_set_style_text_opa},
    {"obj_set_style_text_letter_space", lvgl_obj_set_style_text_letter_space},
    {"obj_set_style_text_line_space", lvgl_obj_set_style_text_line_space},
    {"obj_set_style_text_align", lvgl_obj_set_style_text_align},
    {"obj_set_style_text_decor", lvgl_obj_set_style_text_decor},
    {"obj_set_style_anim_time", lvgl_obj_set_style_anim_time},
    
    // Widget functions
    {"label_create", lvgl_label_create},
    {"label_set_text", lvgl_label_set_text},
    {"label_set_long_mode", lvgl_label_set_long_mode},
    {"btn_create", lvgl_btn_create},
    {"slider_create", lvgl_slider_create},
    {"slider_set_value", lvgl_slider_set_value},
    {"switch_create", lvgl_switch_create},
    {"bar_create", lvgl_bar_create},
    {"bar_set_value", lvgl_bar_set_value},
    {"bar_set_range", lvgl_bar_set_range},
    {"bar_set_mode", lvgl_bar_set_mode},
    {"spangroup_create", lvgl_spangroup_create},
    {"spangroup_new_span", lvgl_spangroup_new_span},
    {"span_set_text", lvgl_span_set_text},
    {"spangroup_set_align", lvgl_spangroup_set_align},
    {"spangroup_set_overflow", lvgl_spangroup_set_overflow},
    {"spangroup_set_mode", lvgl_spangroup_set_mode},
    {"spangroup_refr_mode", lvgl_spangroup_refr_mode},
    {"msgbox_create", lvgl_msgbox_create},
    {"list_create", lvgl_list_create},
    {"list_add_text", lvgl_list_add_text},
    {"list_add_btn", lvgl_list_add_btn},
    
    // Utility functions
    {"color_hex", lvgl_color_hex},
    {"color_white", lvgl_color_white},
    {"color_black", lvgl_color_black},
    {"refr_now", lvgl_refr_now},
    {"scr_load_anim", lvgl_scr_load_anim},
    
    // Font constants
    {"font_montserrat_14", lvgl_font_montserrat_14},
    {"font_montserrat_16", lvgl_font_montserrat_16},
    {"font_montserrat_20", lvgl_font_montserrat_20},
    
    // Animation constants
    {"ANIM_OFF", lvgl_anim_off},
    {"ANIM_ON", lvgl_anim_on},
    
    // Alignment constants
    {"ALIGN_CENTER", lvgl_align_center},
    {"ALIGN_TOP_LEFT", lvgl_align_top_left},
    {"ALIGN_TOP_MID", lvgl_align_top_mid},
    {"ALIGN_TOP_RIGHT", lvgl_align_top_right},
    {"ALIGN_BOTTOM_LEFT", lvgl_align_bottom_left},
    {"ALIGN_BOTTOM_MID", lvgl_align_bottom_mid},
    {"ALIGN_BOTTOM_RIGHT", lvgl_align_bottom_right},
    {"ALIGN_OUT_TOP_MID", lvgl_align_out_top_mid},
    
    // Part constants
    {"PART_MAIN", lvgl_part_main},
    {"PART_INDICATOR", lvgl_part_indicator},
    {"PART_KNOB", lvgl_part_knob},
    
    // State constants
    {"STATE_DEFAULT", lvgl_state_default},
    {"STATE_CHECKED", lvgl_state_checked},
    
    // Border constants
    {"BORDER_SIDE_FULL", lvgl_border_side_full},
    
    // Text alignment constants
    {"TEXT_ALIGN_LEFT", lvgl_text_align_left},
    {"TEXT_ALIGN_CENTER", lvgl_text_align_center},
    {"TEXT_ALIGN_RIGHT", lvgl_text_align_right},
    
    // Scrollbar mode constants
    {"SCROLLBAR_MODE_OFF", lvgl_scrollbar_mode_off},
    
    // Label mode constants
    {"LABEL_LONG_WRAP", lvgl_label_long_wrap},
    
    // Span constants
    {"SPAN_OVERFLOW_CLIP", lvgl_span_overflow_clip},
    {"SPAN_MODE_BREAK", lvgl_span_mode_break},
    
    // Bar mode constants
    {"BAR_MODE_NORMAL", lvgl_bar_mode_normal},
    
    // Grad dir constants
    {"GRAD_DIR_NONE", lvgl_grad_dir_none},
    
    // Text decoration constants
    {"TEXT_DECOR_NONE", lvgl_text_decor_none},
    
    // Screen load animation constants
    {"SCR_LOAD_ANIM_NONE", lvgl_scr_load_anim_none},
    
    {NULL, NULL}
};

static const luaL_Reg lvgl_obj_methods[] = {
    {"__gc", lvgl_obj_gc},
    {"__tostring", lvgl_obj_tostring},
    {NULL, NULL}
};

int luaopen_lvgl(lua_State* L) {
    ESP_LOGI(TAG, "Registering LVGL bindings...");
    
    // Create object metatable
    luaL_newmetatable(L, LVGL_OBJ_METATABLE);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, lvgl_obj_methods, 0);
    lua_pop(L, 1);
    
    // Create lvgl table
    luaL_newlib(L, lvgl_functions);
    
    // Set global lvgl table
    lua_setglobal(L, "lvgl");
    
    ESP_LOGI(TAG, "LVGL bindings registered successfully");
    return 0;
}
